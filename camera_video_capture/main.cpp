#include <stdio.h>
#include <gst/gst.h>
#include <cassert>


//gst-launch-1.0 -v ksvideosrc ! videomixer ! autovideosink sync=false

//gst-launch-1.0 -v videomixer name=mixer ! autovideosink sync=false  ksvideosrc ! mixer. videotestsrc pattern=18 ! mixer.

//gst-launch-1.0 -v videomixer name=mixer sink_1::xpos=800 ! autovideosink sync=false  ksvideosrc ! video/x-raw,format=BGR,width=800,height=600 ! videoconvert  ! mixer. videotestsrc pattern=18 ! mixer.

//gst-launch-1.0 -v videomixer name=mixer sink_1::xpos=800 sink_2::xpos=800 sink_2::ypos=300 ! videoconvert ! vp8enc ! webmmux ! filesink location=D:/demo.webm  ksvideosrc ! video/x-raw,format=BGR,width=800,height=600 ! videoconvert  ! mixer. videotestsrc pattern=18 ! mixer. videotestsrc pattern=0 ! mixer.


//logo
//gst-launch-1.0 -v filesrc location=D:/hopit-logo.png ! pngdec ! imagefreeze !videomixer ! autovideosink

//gst-launch-1.0 -v videomixer name=mixer ! autovideosink sync=false ksvideosrc ! mixer. filesrc location=D:/hopit-logo.png ! decodebin ! imagefreeze ! mixer.

//gst-launch-1.0 -v videomixer name=mixer sink_1::xpos=800 sink_2::xpos=800 sink_2::ypos=300 ! autovideosink sync=false  ksvideosrc ! video/x-raw,format=BGR,width=800,height=600 ! videoconvert  ! mixer. videotestsrc pattern=18 ! mixer. videotestsrc pattern=0 ! mixer.
//gst-launch-1.0 -v ksvideosrc !videoconvert !rawvideoparse width=640 height=480 framerate=25/1 format=16 ! videomixer ! autovideosink



//gst-launch-1.0 -v ksvideosrc !rawvideoparse width=640 height=480 framerate=25/1 format=16 !videoconvert ! autovideosink
//it runs smooth

//gst-launch-1.0 -v ksvideosrc !videoconvert !rawvideoparse width=640 height=480 framerate=25/1 format=16 ! videomixer  !vp8enc !webmmux ! filesink location=D:/dump.webm


//http://gstreamer-devel.966125.n4.nabble.com/Still-can-t-get-videomixer-to-do-live-video-at-full-frame-rate-td3748457.html
//http://gstreamer-devel.966125.n4.nabble.com/v4l2rc-MJPEG-to-Videomixer-slow-alternative-pathways-td4684534.html
//https://gstreamer.freedesktop.org/documentation/vpx/vp8enc.html?gi-language=c#vp8enc


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


static void handler(GstElement *src, GstPad *new_pad, void* ptr)
{
	assert(false);
	//GstPad *sink_pad = gst_element_get_static_pad(data->convert, "sink");
	GstPadLinkReturn ret;
	GstCaps *new_pad_caps = NULL;
	GstStructure *new_pad_struct = NULL;
	const gchar *new_pad_type = NULL;

	//if (gst_pad_is_linked(sink_pad))
	//{
	//	g_print("we are linked. igonring\n");
	//}
	// check the new pad types
	// we have previously created a piece of pipeline which deals with videoconvert linked with xvimagesink and we will nto be able to link it to a pad producing video.
	//gst-pad_get_current_caps()- retrieves current capabilities of pad 
	new_pad_caps = gst_pad_get_current_caps(new_pad);
	new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
	new_pad_type = gst_structure_get_name(new_pad_struct);

	if (!g_str_has_prefix(new_pad_type, "video/x-raw"))
	{
		g_print("It has new pad type");
	}

	// gst_pad_link tries to link two pads . the link must be specified from source to sink and both pads must be owned by elements residing in same pipeline
	//ret = gst_pad_link(new_pad, sink_pad);
	//if (GST_PAD_LINK_FAILED(ret))
	//{
	//	g_print("type is new_pad_type");
	//}
	//if (new_pad_caps != NULL)
	//{
	//	gst_caps_unref(new_pad_caps);
	//}
	//gst_object_unref(sink_pad);
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
	GstElement *source1 = gst_element_factory_make("ksvideosrc", "source1");
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
	//GstElement * clrspace = gst_element_factory_make("videoconvert", "clrspace");
	//assert(clrspace);

	//GstElement * clrspace1 = gst_element_factory_make("videoconvert", "clrspace1");
	//assert(clrspace1);
	
	GstElement *sink = gst_element_factory_make("autovideosink", "sink");
	assert(sink);
	g_object_set(G_OBJECT(sink), "sync", false, NULL);

	/*GstElement *sink = gst_element_factory_make("filesink", "sink-file");
	assert(sink);
	g_object_set(G_OBJECT(sink), "location", "dump.webm", NULL);*/

	
	GstElement* container = gst_element_factory_make("webmmux", NULL);
	assert(container);
	GstElement* enocoder = gst_element_factory_make("vp8enc", NULL );
	assert(enocoder);

	GstElement* fileSrc = gst_element_factory_make("filesrc", NULL);
	assert(fileSrc);

	g_object_set(G_OBJECT(fileSrc), "location", "D:/logo.png", NULL);

	GstElement *pngDec = gst_element_factory_make("pngdec", NULL);
	assert(pngDec);

	GstElement* imageFreez = gst_element_factory_make("imagefreeze", NULL);
	assert(imageFreez);

	//GstElement* queue = gst_element_factory_make("queue", NULL);
	//assert(queue);

	//g_object_set(G_OBJECT(queue), "max-size-time", 0, "max-size-buffers", 0, "max-size-bytes", 0,
	//	"leaky", 1, NULL);

//	GstElement* videorate = gst_element_factory_make("videorate", NULL);
	//assert(videorate);
	//g_object_set(G_OBJECT(videorate), "in", 25, "out", 25,NULL);
	if (!pipeline || !source1 || !source2 || !sink) {
		g_printerr("One element could not be created. Exiting.\n");
		return -1;
	}

	const int cam_video_w = 800;
	const int cam_video_h = 600;
	GstCaps *filtercaps = gst_caps_new_simple("video/x-raw",
		"format", G_TYPE_STRING, "I420",
		"width", G_TYPE_INT, 800,
		"height", G_TYPE_INT, 600,
		NULL);
	assert(filtercaps);

	g_object_set(G_OBJECT(filter1), "caps", filtercaps, NULL);
	gst_caps_unref(filtercaps);
	
	GstCaps *Cameracaps = gst_caps_new_simple("video/x-raw",
		"format", G_TYPE_STRING, "I420",
		"width", G_TYPE_INT, cam_video_w,
		"height", G_TYPE_INT, cam_video_h,
		"framerate", GST_TYPE_FRACTION, 25, 1,
		NULL);
	g_object_set(G_OBJECT(filter), "caps", Cameracaps, NULL);
	
	gst_caps_unref(Cameracaps);

	
	/* Set up the pipeline */

	/* we set the input filename to the source element */
	//g_object_set(G_OBJECT(source1), "pattern", 18, NULL); //circle pattern
	g_object_set(G_OBJECT(source2), "pattern", 18, NULL);  // snow pattern

	/* we add a message handler */
	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	assert(bus);
	const auto r = gst_bus_add_watch(bus, bus_call, loop);
	assert(r > 0);
	gst_object_unref(bus);

	/* we add all elements into the pipeline */
	gst_bin_add_many(GST_BIN(pipeline),
		source1, filter, filter1, mixer, sink, source2, /*enocoder, container,*/ 
		/*queue, timeoverlay, videorate,*/
		fileSrc, pngDec, 
		imageFreez,NULL);

	/* Manually link the mixer, which has "Request" pads */
	
		auto* mixer_sink_pad_template = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(mixer), "sink_%u");
		assert(mixer_sink_pad_template);
		{
		auto* mixer_sink_pad = gst_element_request_pad(mixer, mixer_sink_pad_template, NULL, NULL);
		assert(mixer_sink_pad);
		//auto* name = gst_pad_get_name(mixer_sink_pad);
		auto * sink_pad = gst_element_get_static_pad(filter1, "src");
		assert(sink_pad);
		{
			const auto r = gst_pad_link(sink_pad, mixer_sink_pad);
			assert(r == 0);
		}
		g_object_set(mixer_sink_pad, "xpos", cam_video_w, NULL);//source2 for checker
		
		gst_object_unref(GST_OBJECT(mixer_sink_pad));
		gst_object_unref(GST_OBJECT(sink_pad));
		}
	
		{
			auto* mixer_sink_pad1 = gst_element_request_pad(mixer, mixer_sink_pad_template, NULL, NULL);
			assert(mixer_sink_pad1);
			auto * sink_pad1 = gst_element_get_static_pad(filter, "src");
			
			assert(sink_pad1);
			{
				const auto r = gst_pad_link(sink_pad1, mixer_sink_pad1);
				assert(r == 0);
			}
			g_object_set(mixer_sink_pad1, "xpos", 0, NULL);
			gst_object_unref(GST_OBJECT(mixer_sink_pad1));
			gst_object_unref(GST_OBJECT(sink_pad1));
		}

		{
			auto* mixer_sink_pad1 = gst_element_request_pad(mixer, mixer_sink_pad_template, NULL, NULL);
			assert(mixer_sink_pad1);
			auto * sink_pad1 = gst_element_get_static_pad(imageFreez, "src");

			assert(sink_pad1);
			{
				const auto r = gst_pad_link(sink_pad1, mixer_sink_pad1);
				assert(r == 0);
			}
			g_object_set(mixer_sink_pad1, "xpos", 0, NULL);
			gst_object_unref(GST_OBJECT(mixer_sink_pad1));
			gst_object_unref(GST_OBJECT(sink_pad1));
		}

		gst_element_link_many(mixer,/* enocoder, container, */sink, NULL);
		
		gst_element_link_many(source1,  filter, mixer, NULL);



	
		gst_element_link_many(source2, filter1, /*clrspace,*/ NULL);
		gst_element_link_many(fileSrc, pngDec, imageFreez, NULL);

	
	
	
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

