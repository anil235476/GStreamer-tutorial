#include <stdio.h>
#include <gst/gst.h>
#include <cassert>


//https://stackoverflow.com/questions/8721527/how-to-program-videomixer-using-gstreamer-c-api
//xvimagesink not work for windows
//intead https://stackoverflow.com/questions/20614634/xvimagesink-not-found-by-gst-inspect
//autovideosink plugin is used.
//o element “ffmpegcolorspace” in GStreamer
//now renamed as videoconvert
//https://stackoverflow.com/questions/44780991/no-element-ffmpegcolorspace-in-gstreamer
//#include <gst/gst.h>
//#include <glib.h>


static gboolean
bus_call(GstBus     *bus,
	GstMessage *msg,
	gpointer    data)
{
	GMainLoop *loop = (GMainLoop *)data;

	switch (GST_MESSAGE_TYPE(msg)) {

	case GST_MESSAGE_EOS:
		g_print("End of stream\n");
		g_main_loop_quit(loop);
		break;

	case GST_MESSAGE_ERROR: {
		gchar  *debug;
		GError *error;

		gst_message_parse_error(msg, &error, &debug);
		g_free(debug);

		g_printerr("Error: %s\n", error->message);
		g_error_free(error);

		g_main_loop_quit(loop);
		break;
	}
	default:
		break;
	}

	return TRUE;
}


int
main(int   argc,
	char *argv[])
{

	/* Initialisation */
	gst_init(&argc, &argv);

	GMainLoop* loop = g_main_loop_new(NULL, FALSE);

	assert(loop);
	

	 /* Create gstreamer elements */
	GstElement *pipeline = gst_pipeline_new("player");
	assert(pipeline);
	GstElement *source1 = gst_element_factory_make("videotestsrc", "source1");
	assert(source1);
	GstElement *source2 = gst_element_factory_make("videotestsrc", "source2");
	assert(source2);
	/*
	GstElement *scale = gst_element_factory_make("videoscale", "scale");
	assert(scale);*/
	GstElement *filter = gst_element_factory_make("capsfilter", "filter");
	assert(filter);
	GstElement *filter1 = gst_element_factory_make("capsfilter", "filter1");
	assert(filter1);
	
	GstElement *mixer = gst_element_factory_make("videomixer", "mixer");
	assert(mixer);
	GstElement * clrspace = gst_element_factory_make("videoconvert", "clrspace");
	assert(clrspace);

	GstElement * clrspace1 = gst_element_factory_make("videoconvert", "clrspace1");
	assert(clrspace1);
	
	GstElement *sink = gst_element_factory_make("autovideosink", "sink");
	assert(sink);

	
	if (!pipeline || !source1 || !source2 || !sink) {
		g_printerr("One element could not be created. Exiting.\n");
		return -1;
	}

	GstCaps *filtercaps = gst_caps_new_simple("video/x-raw",
		"format", G_TYPE_STRING, "I420",
		"width", G_TYPE_INT, 800,
		"height", G_TYPE_INT, 600,
		NULL);
	assert(filtercaps);
	g_object_set(G_OBJECT(filter), "caps", filtercaps, NULL);
	gst_caps_unref (filtercaps);

	GstCaps *filtercaps1 = gst_caps_new_simple("video/x-raw",
		"format", G_TYPE_STRING, "I420",
		"width", G_TYPE_INT, 800,
		"height", G_TYPE_INT, 600,
		NULL);
	assert(filtercaps1);
	g_object_set(G_OBJECT(filter1), "caps", filtercaps1, NULL);
	gst_caps_unref(filtercaps1);

	
	/* Set up the pipeline */

	/* we set the input filename to the source element */
	g_object_set(G_OBJECT(source1), "pattern", 11, NULL); //circle pattern
	g_object_set(G_OBJECT(source2), "pattern", 0, NULL);  // snow pattern

	/* we add a message handler */
	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	assert(bus);
	const auto r = gst_bus_add_watch(bus, bus_call, loop);
	assert(r > 0);
	gst_object_unref(bus);

	/* we add all elements into the pipeline */
	gst_bin_add_many(GST_BIN(pipeline),
		source1, filter,filter1, /* videobox1,*/  mixer,  clrspace, clrspace1, sink, source2,/* videobox2,*/ NULL);

	/* Manually link the mixer, which has "Request" pads */
	
		auto* mixer_sink_pad_template = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(mixer), "sink_%u");
		assert(mixer_sink_pad_template);
		{
		auto* mixer_sink_pad = gst_element_request_pad(mixer, mixer_sink_pad_template, NULL, NULL);
		assert(mixer_sink_pad);
		//auto* name = gst_pad_get_name(mixer_sink_pad);
		auto * sink_pad = gst_element_get_static_pad(clrspace, "src");
		assert(sink_pad);
		{
			const auto r = gst_pad_link(sink_pad, mixer_sink_pad);
			assert(r == 0);
		}
		g_object_set(mixer_sink_pad, "xpos", 800, NULL);//source2 for checker
		
		gst_object_unref(GST_OBJECT(mixer_sink_pad));
		gst_object_unref(GST_OBJECT(sink_pad));
		}
	
		{
			auto* mixer_sink_pad1 = gst_element_request_pad(mixer, mixer_sink_pad_template, NULL, NULL);
			assert(mixer_sink_pad1);
			auto * sink_pad1 = gst_element_get_static_pad(clrspace1, "src");
			
			assert(sink_pad1);
			{
				const auto r = gst_pad_link(sink_pad1, mixer_sink_pad1);
				assert(r == 0);
			}
			g_object_set(mixer_sink_pad1, "xpos", 0, NULL);
			gst_object_unref(GST_OBJECT(mixer_sink_pad1));
			gst_object_unref(GST_OBJECT(sink_pad1));
		}
		gst_element_link_many(mixer, sink, NULL);
		gst_element_link_many(source2, filter1, clrspace,NULL);
		gst_element_link_many(source1, filter, clrspace1,  NULL);
	
	
	
	
	/* Set the pipeline to "playing" state*/
	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	/* Iterate */
	g_print("Running...\n");
	g_main_loop_run(loop);

	/* Out of the main loop, clean up nicely */
	g_print("Returned, stopping playback\n");
	gst_element_set_state(pipeline, GST_STATE_NULL);

	g_print("Deleting pipeline\n");
	gst_object_unref(GST_OBJECT(pipeline));

	return 0;
}

