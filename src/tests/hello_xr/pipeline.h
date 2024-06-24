#ifndef HELLO_XR_PIPELINE_H
#define HELLO_XR_PIPELINE_H

#include "pch.h"
#include <opencv2/core.hpp>
#include <gst/gst.h>
#include <gio/gio.h>
#include <gst/gstbus.h>
#include <gst/app/gstappsink.h>
#include <deque>
#include <atomic>
#include <thread>
#include <mutex>
#include <iostream>
#include <string>
#include <memory>
#include <vector>


#include "nativelib/static_gstreamer.h"

namespace quest_teleop {
    enum class StreamType {
        Mono,
        Stereo
    };
    
    enum class CodecType {
        H264,
        H265,
        AV1
    };
    
    enum class PipelineSide {
        Left=0,
        Right=1,
        Both
    };

    struct StreamConfig {
        StreamType type;
        PipelineSide side;
        CodecType codec;
        int port;
        XrVector3f position;
        XrVector3f scale;
        std::string name;
        int width;
        int height;
    };
    
    struct SampleRead {
        // delete copy constructor
        SampleRead(const SampleRead &) = delete;

        // delete copy assignment
        SampleRead &operator=(const SampleRead &) = delete;

        SampleRead() {
            images[0] = cv::Mat(cv::Size(10, 10), CV_8UC3,
                            cv::Scalar(0, 0, 200));
            images[1] = cv::Mat(cv::Size(10, 10), CV_8UC3,
                            cv::Scalar(0, 0, 200));
        }

        ~SampleRead() {
            if (buffer && mapped) {
                gst_buffer_unmap(buffer, &info);
            }
            if (sample) {
                gst_sample_unref(sample);
            }
        }

        cv::Mat images[2];
        GstMapInfo info;
        GstBuffer *buffer = nullptr;
        GstSample *sample = nullptr;
        bool mapped = false;
    };
    
    class Pipeline {
        //delete copy constructor
        Pipeline(const Pipeline &) = delete;

        // delete copy assignment
        Pipeline &operator=(const Pipeline &) = delete;

    public:
        static void InitializeGStreamer();

        Pipeline(const StreamConfig &streamConfig);

        ~Pipeline();

        void getPadProperty(GstElement *element, const gchar *pad_name, const gchar *property_name,
                            int *value);

        void SetVideoSize();       

        void SampleReader();

        cv::Mat &GetImage(PipelineSide side=PipelineSide::Left);

        StreamType GetPipelineType() const {
            return m_streamConfig.type;
        }
        
        PipelineSide GetPipelineSide() const {
            return m_streamConfig.side;
        }
        
        CodecType GetPipelineCodec() const {
            return m_streamConfig.codec;
        }
        
        int GetPipelinePort() const {
            return m_streamConfig.port;
        }
        
        XrVector3f GetPipelinePosition() const {
            return m_streamConfig.position;
        }
        
        XrVector3f GetPipelineScale() const {
            return m_streamConfig.scale;
        }
        
        std::string GetPipelineName() const {
            return m_streamConfig.name;
        }    

    private:
        std::deque <SampleRead> m_samples;
        static bool is_initialized;
        GstElement *m_pipeline;
        GstBus *m_bus;
        GstMessage *m_msg;
        GstAppSink *m_appSink;
        GMainContext *m_dataContext;
        std::atomic<bool> m_exit{false};
        std::unique_ptr <std::thread> m_thread;
        std::mutex m_mutex;
        static const int kMaxSamples = 10;

        GstElement *m_vc_factory;

        int m_width;
        int m_height;   
        StreamConfig m_streamConfig;
    };
}   // namespace quest_teleop

#endif //HELLO_XR_PIPELINE_H
