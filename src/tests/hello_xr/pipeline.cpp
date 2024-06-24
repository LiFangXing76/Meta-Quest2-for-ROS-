#include "pipeline.h"

#include <opencv2/imgproc.hpp>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/gstbus.h>
#include "logger.h"
#include "nativelib/static_gstreamer.h"
#include "common.h"

namespace quest_teleop {
    namespace {
        /* Functions below print the Capabilities in a human-friendly format */
        static gboolean print_field(GQuark field, const GValue *value, gpointer pfx) {
            gchar *str = gst_value_serialize(value);

            Log::Write(Log::Level::Info,
                       std::string((gchar *) pfx) + "   " + std::string(g_quark_to_string(field)) +
                       ": " + std::string(str));
            g_free(str);
            return TRUE;
        }

        static void print_caps(const GstCaps *caps, const gchar *pfx) {
            guint i;

            g_return_if_fail(caps != NULL);

            if (gst_caps_is_any(caps)) {
                Log::Write(Log::Level::Info, std::string(pfx) + "ANY");
                return;
            }
            if (gst_caps_is_empty(caps)) {
                Log::Write(Log::Level::Info, std::string(pfx) + "EMPTY");
                return;
            }

            for (i = 0; i < gst_caps_get_size(caps); i++) {
                GstStructure *structure = gst_caps_get_structure(caps, i);

                Log::Write(Log::Level::Info,
                           std::string(pfx) + std::string(gst_structure_get_name(structure)));
                gst_structure_foreach(structure, print_field, (gpointer) pfx);
            }
        }

/* Prints information about a Pad Template, including its Capabilities */
        static void print_pad_templates_information(GstElementFactory *factory) {
            const GList *pads;
            GstStaticPadTemplate *padtemplate;

            Log::Write(Log::Level::Info,
                       "Pad Templates for " +
                       std::string(gst_element_factory_get_longname(factory)));
            if (!gst_element_factory_get_num_pad_templates(factory)) {
                Log::Write(Log::Level::Info, "  none");
                return;
            }

            pads = gst_element_factory_get_static_pad_templates(factory);
            while (pads) {
                padtemplate = (GstStaticPadTemplate *) pads->data;
                pads = g_list_next(pads);

                if (padtemplate->direction == GST_PAD_SRC)
                    Log::Write(Log::Level::Info,
                               "  SRC template: '" + std::string(padtemplate->name_template) +
                               "'\n");
                else if (padtemplate->direction == GST_PAD_SINK)
                    Log::Write(Log::Level::Info,
                               "  SINK template: '" + std::string(padtemplate->name_template) +
                               "'\n");
                else
                    Log::Write(Log::Level::Info,
                               "  UNKNOWN!!! template: '" +
                               std::string(padtemplate->name_template) +
                               "'\n");

                if (padtemplate->presence == GST_PAD_ALWAYS)
                    Log::Write(Log::Level::Info, "    Availability: Always\n");
                else if (padtemplate->presence == GST_PAD_SOMETIMES)
                    Log::Write(Log::Level::Info, "    Availability: Sometimes\n");
                else if (padtemplate->presence == GST_PAD_REQUEST)
                    Log::Write(Log::Level::Info, "    Availability: On request\n");
                else
                    Log::Write(Log::Level::Info, "    Availability: UNKNOWN!!!\n");

                if (padtemplate->static_caps.string) {
                    GstCaps *caps;
                    Log::Write(Log::Level::Info, "    Capabilities:\n");
                    caps = gst_static_caps_get(&padtemplate->static_caps);
                    print_caps(caps, "      ");
                    gst_caps_unref(caps);

                }
            }
        }

/* Shows the CURRENT capabilities of the requested pad in the given element */
        static void print_pad_capabilities(GstElement *element, gchar *pad_name) {
            GstPad *pad = NULL;
            GstCaps *caps = NULL;

            /* Retrieve pad */
            pad = gst_element_get_static_pad(element, pad_name);
            if (!pad) {
                Log::Write(Log::Level::Error,
                           "Could not retrieve pad '" + std::string(pad_name) + "'");
                return;
            }

            /* Retrieve negotiated caps (or acceptable caps if negotiation is not finished yet) */
            caps = gst_pad_get_current_caps(pad);
            if (!caps)
                caps = gst_pad_query_caps(pad, NULL);

            /* Print and free */
            Log::Write(Log::Level::Info, "Caps for the " + std::string(pad_name) + "pad:");
            print_caps(caps, "      ");
            gst_caps_unref(caps);
            gst_object_unref(pad);
        }
    } // namespace

    bool Pipeline::is_initialized = false;
    
    void Pipeline::InitializeGStreamer() {
        if (!is_initialized) {
            gst_init_static_plugins();
            Log::Write(Log::Level::Verbose, "Initializing gstreamer");
            gst_init(nullptr, nullptr);
            is_initialized = true;
        }
    }

    Pipeline::Pipeline(const StreamConfig &streamConfig) : m_streamConfig{streamConfig}, m_width{-1}, m_height{-1} {
        InitializeGStreamer();
        
        // Onstructing pipeline string
        auto portStr = std::to_string(m_streamConfig.port);
        std::string pipeline = "udpsrc port=" + portStr + " caps=\"application/x-rtp,media=video,clock-rate=90000,payload=96,encoding-name=H264\" ! rtph264depay ! decodebin3 ! videoconvert name=videoconvert"+ portStr +" ! video/x-raw,format=RGB ! appsink name=appsink" + portStr;

        Log::Write(Log::Level::Info, "Created context");
        m_dataContext = g_main_context_new();
        g_main_context_push_thread_default(m_dataContext);
        GError *error = nullptr;
        m_pipeline = gst_parse_launch(pipeline.c_str(), &error);
        Log::Write(Log::Level::Info, "Checking the pipeline");
        if (error) {
            Log::Write(Log::Level::Info, "pipeline not created");
            gchar *message = g_strdup_printf("Unable to build pipeline: %s", error->message);
            g_clear_error(&error);
            Log::Write(Log::Level::Error, message
            );
            g_free(message);
        }

        Log::Write(Log::Level::Info, "Getting the appsink");
        std::string appSinkStr = "appsink" + portStr;
        m_appSink = (GstAppSink * )(
                gst_bin_get_by_name((GstBin * )(m_pipeline), appSinkStr.c_str()));
        if (!m_appSink) {
            Log::Write(Log::Level::Error, "couldn't find appsink");
        }

        std::string videoConvertStr = "videoconvert" + portStr;
        m_vc_factory =
                gst_bin_get_by_name((GstBin * )(m_pipeline), videoConvertStr.c_str());
        if (!m_vc_factory) {
            Log::Write(Log::Level::Error,
                       "Couldn't find videoconvert factory element");
        } else {
            Log::Write(Log::Level::Info,
                       "Pad capabilities before receiving the stream.");
            print_pad_capabilities(m_vc_factory,
                                   "sink");
        }

        Log::Write(Log::Level::Info, "Setting pipeline to playing");
/* Start playing */
        auto ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            Log::Write(Log::Level::Error,
                       "Unable to set the pipeline to the playing state.");
        } else {
            Log::Write(Log::Level::Info, "Pipeline is playing");

            Log::Write(Log::Level::Info, "First query");
/* Wait until error or EOS */
            m_bus = gst_element_get_bus(m_pipeline);
            m_msg = gst_bus_timed_pop_filtered(m_bus, 10000000,
                                               (GstMessageType)(GST_MESSAGE_ERROR |
                                                                GST_MESSAGE_EOS));
            if (m_msg) {
                if (
                        GST_MESSAGE_TYPE(m_msg)
                        == GST_MESSAGE_ERROR) {
                    Log::Write(Log::Level::Warning, "An error occurred! ");
                }
                GError *err;
                gchar *debug_info;

                if (
                        GST_MESSAGE_TYPE(m_msg)
                        == GST_MESSAGE_ERROR) {
                    gst_message_parse_error(m_msg, &err, &debug_info
                    );
                    Log::Write(Log::Level::Warning,
                               std::string("Error received from element ")
                               +
                               std::string(GST_OBJECT_NAME(m_msg->src)
                               ) +
                               std::string(err
                                                   ->message));

                    Log::Write(Log::Level::Warning,
                               std::string("Debugging information: ")
                               +
                               std::string(
                                       debug_info
                                       ? debug_info : "none"));
                    g_clear_error(&err);
                    g_free(debug_info);
                }
                gst_message_unref(m_msg);
            }

            m_thread = std::make_unique<std::thread>(&Pipeline::SampleReader, this);
        }
    }    

    Pipeline::~Pipeline() {
        m_exit = true;
        if (m_thread) {
            m_thread->join();
        }

        /* Free resources */

        if (m_msg) {
            gst_message_unref(m_msg);
        }
        if (m_bus) {
            gst_object_unref(m_bus);
        }

        if (m_vc_factory) {
            gst_object_unref(m_vc_factory);
        }
        g_main_context_pop_thread_default(m_dataContext);
        g_main_context_unref(m_dataContext);
        if (m_pipeline) {
            gst_element_set_state(m_pipeline, GST_STATE_NULL);
            if (m_appSink) {
                gst_object_unref(m_appSink);
            }
            gst_object_unref(m_pipeline);
        }
    }

    void Pipeline::getPadProperty(GstElement *element, const gchar *pad_name, const gchar *property_name, int *value) {
        GstPad *pad = gst_element_get_static_pad(element, pad_name);
        if (!pad) {
            Log::Write(Log::Level::Error,
                       "Could not retrieve pad '" + std::string(pad_name) + "'");
            return;
        }
        GstCaps *caps = gst_pad_get_current_caps(pad);
        if (!caps) {
            caps = gst_pad_query_caps(pad, NULL);
        }
        if (!caps) {
            Log::Write(Log::Level::Error,
                       "Could not retrieve caps for pad '" + std::string(pad_name) +
                       "'");
            return;
        }
        GstStructure *structure = gst_caps_get_structure(caps, 0);
        if (!structure) {
            Log::Write(Log::Level::Error, "Could not retrieve structure for pad '" +
                                          std::string(pad_name) + "'");
            return;
        }
        if (!gst_structure_get_int(structure, property_name, value)) {
            Log::Write(Log::Level::Error,
                       "Could not retrieve property '" + std::string(property_name) +
                       "'");
        }
        gst_caps_unref(caps);
        gst_object_unref(pad);
    }

    void Pipeline::SetVideoSize() {
        print_pad_capabilities(m_vc_factory, "sink");
        getPadProperty(m_vc_factory, "sink", "width", &m_width);
        getPadProperty(m_vc_factory, "sink", "height", &m_height);
    }

    void Pipeline::SampleReader() {
        while (!m_exit) {
            TimeRecorder timeRecorder = TimeRecorder(true);

            m_msg = gst_bus_pop_filtered(m_bus,
                                         (GstMessageType)(GST_MESSAGE_ERROR |
                                                          GST_MESSAGE_EOS));

            timeRecorder.LogElapsedTime("Pop filter returned after ");
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_samples.size() > kMaxSamples) {
                    m_samples.pop_front();
                }
                m_samples.emplace_back();
            }
            timeRecorder.LogElapsedTime("Locking took ");

            SampleRead &sampleRead = m_samples.back();
            if (m_msg) {
                /* See next tutorial for proper error message handling/parsing */
                if (GST_MESSAGE_TYPE(m_msg) == GST_MESSAGE_ERROR) {
                    Log::Write(Log::Level::Error,
                               "An error occurred! Re-run with the GST_DEBUG=*:WARN environment "
                               "variable set for more details.");
                }
                for (int i = 0; i < 2; ++i) {
                    sampleRead.images[i] = cv::Mat(cv::Size(m_streamConfig.width, m_streamConfig.height), CV_8UC3,
                                                   cv::Scalar(255, 255, 0));
                    cv::putText(sampleRead.images[i], "Error or End Video",
                                cv::Point(m_streamConfig.width/2, m_streamConfig.height/2),
                                cv::FONT_HERSHEY_SIMPLEX, 5, cv::Scalar(255, 0, 0), 4,
                                cv::LINE_AA);
                }

                gst_message_unref(m_msg);
            } else {
                Log::Write(Log::Level::Info, "Getting sample");
                sampleRead.sample = gst_app_sink_pull_sample(m_appSink);
                timeRecorder.LogElapsedTime("Getting a sample took ");
                if (sampleRead.sample) {
                    Log::Write(Log::Level::Info, "getting buffer");
                    GstBufferList *bufferList = gst_sample_get_buffer_list(sampleRead.sample);
                    timeRecorder.LogElapsedTime("Getting a buffer list took ");
                    if (bufferList) {
                        auto totalSize = gst_buffer_list_calculate_size(bufferList);
                        auto length = gst_buffer_list_length(bufferList);
                        std::stringstream ss;
                        ss << "Total size of the sample is: " << totalSize
                           << ". Length of the list is: " << length << ".";
                        Log::Write(Log::Level::Info, ss.str());
                    }
                    sampleRead.buffer = gst_sample_get_buffer(sampleRead.sample);
                    timeRecorder.LogElapsedTime("Getting a buffer sample took ");
                    if (sampleRead.buffer) {
                        Log::Write(Log::Level::Info, "mapping");

                        sampleRead.mapped = gst_buffer_map(sampleRead.buffer, &sampleRead.info,
                                                           GST_MAP_READ);
                        timeRecorder.LogElapsedTime("Mapping buffer took ");
                        if (sampleRead.mapped) {
                            const auto str_size =
                                    std::string("Size is") +
                                    std::to_string(sampleRead.info.size);

                            if (m_width == -1) {
                                Log::Write(Log::Level::Warning,
                                           "Width and height not set");
                                SetVideoSize();
                                // Log the width and height
                                Log::Write(Log::Level::Info,
                                           "Width: " + std::to_string(m_width) +
                                           " Height: " + std::to_string(m_height));
                            }

                            if (sampleRead.info.size != m_width * m_height * 3) {
                                throw std::runtime_error(
                                        "Size of the buffer is not as expected");
                            }
                            if (m_streamConfig.type == StreamType::Mono || m_streamConfig.side != PipelineSide::Both) {
                                sampleRead.images[static_cast<int>(m_streamConfig.side)] = cv::Mat(cv::Size(m_width, m_height), CV_8UC3,
                                                           sampleRead.info.data);

                            } else {
                                cv::Mat image = cv::Mat(cv::Size(m_width, m_height), CV_8UC3,
                                                        sampleRead.info.data);
                                sampleRead.images[static_cast<int>(PipelineSide::Left)] = image(cv::Rect(0, 0, m_width / 2, m_height));
                                sampleRead.images[static_cast<int>(PipelineSide::Right)] = image(cv::Rect(m_width / 2, 0, m_width / 2, m_height));
                            }

                        }
                    }
                }

                if (!sampleRead.sample || !sampleRead.buffer || !sampleRead.mapped) {
                    sampleRead.images[static_cast<int>(PipelineSide::Left)] = cv::Mat(cv::Size(m_streamConfig.width, m_streamConfig.height), CV_8UC3,
                                               cv::Scalar(0, 0, 200));
                    sampleRead.images[static_cast<int>(PipelineSide::Right)] = cv::Mat(cv::Size(m_streamConfig.width, m_streamConfig.height), CV_8UC3,
                                                   cv::Scalar(0, 0, 200));
                    cv::putText(sampleRead.images[static_cast<int>(PipelineSide::Left)], "[left]"+ m_streamConfig.name,
                                cv::Point(250, 250),
                                cv::FONT_HERSHEY_SIMPLEX, 5, cv::Scalar(255, 0, 0), 4,
                                cv::LINE_AA);
                    cv::putText(sampleRead.images[static_cast<int>(PipelineSide::Right)], "[right]"+ m_streamConfig.name,
                                cv::Point(250, 250),
                                cv::FONT_HERSHEY_SIMPLEX, 5, cv::Scalar(255, 0, 0), 4,
                                cv::LINE_AA);
                }
            }

        }
    }

    cv::Mat &Pipeline::GetImage(PipelineSide side) {
        Log::Write(Log::Level::Info,m_streamConfig.name + " Getting image");
        if (m_streamConfig.side != PipelineSide::Both) {
            side = m_streamConfig.side;
        }
        TimeRecorder timeRecorder = TimeRecorder(true);
        std::lock_guard<std::mutex> lock(m_mutex);
        timeRecorder.LogElapsedTime("Locking in getImage took ");
        if (m_samples.size() <= 1) {
            m_samples.emplace_front();
            SampleRead &sample = m_samples.front();
            sample.images[static_cast<int>(PipelineSide::Left)] = cv::Mat(cv::Size(m_streamConfig.width, m_streamConfig.height), CV_8UC3,
                                   cv::Scalar(0, 0, 200));
            sample.images[static_cast<int>(PipelineSide::Right)] = cv::Mat(cv::Size(m_streamConfig.width, m_streamConfig.height), CV_8UC3,
                                      cv::Scalar(0, 0, 200));
            cv::putText(sample.images[static_cast<int>(PipelineSide::Left)], "[left]"+ m_streamConfig.name,
                        cv::Point(250, 250),
                        cv::FONT_HERSHEY_SIMPLEX, 5, cv::Scalar(255, 0, 0), 4,
                        cv::LINE_AA);
            cv::putText(sample.images[static_cast<int>(PipelineSide::Right)], "[right]"+ m_streamConfig.name,
                        cv::Point(250, 250),
                        cv::FONT_HERSHEY_SIMPLEX, 5, cv::Scalar(255, 0, 0), 4,
                        cv::LINE_AA);
        }
        while (m_samples.size() > 2 && ((side == PipelineSide::Left && m_streamConfig.side == PipelineSide::Both) || m_streamConfig.side != PipelineSide::Both)) {
            m_samples.pop_front();
        }
        auto &image = m_samples.front().images[static_cast<int>(side)];
        return image;
    }
} // namespace quest_teleop
