#include <stdio.h>
#include <gst/gst.h>
#include <cassert>
#include <thread>
#include <future>

//link explains appsrc push buffer mode.
//https://gstreamer.freedesktop.org/documentation/tutorials/basic/short-cutting-the-pipeline.html?gi-language=c
//https://gstreamer.freedesktop.org/documentation/application-development/advanced/pipeline-manipulation.html?gi-language=c#appsrc-example


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


bool pushData = false;
static GstClock *theclock = nullptr;
GstElement* gPipeLine = nullptr;
static gboolean
_read_data(GstElement* app)
{
	//auto* app = f.get();
	const int CHUNK_SIZE = 385 * 288 * 3;;
	uint8_t white_frame[CHUNK_SIZE] = { 0, };
	uint8_t black_black[CHUNK_SIZE] = { 0xff, };
	memset(black_black, 0xff, CHUNK_SIZE);
	while (pushData) {
		static gboolean white = FALSE;
		static GstClockTime timestamp = 0;


		//GstBuffer* buffer = gst_buffer_new();
	//	GstMemory* memory = gst_allocator_alloc(NULL, 100 * 100 * 3, NULL);
	//	gst_buffer_insert_memory(buffer, -1, memory);

		/* Create a new empty buffer */
		GstBuffer* buffer = gst_buffer_new_allocate(NULL, CHUNK_SIZE, NULL);
		assert(buffer);
		/* this makes the image black/white */
		//gst_buffer_memset(buffer, 0, white ? 0xff : 0x0, CHUNK_SIZE);
		auto* ptr = white ? white_frame : black_black;
		gst_buffer_fill(buffer, 0, ptr, CHUNK_SIZE);
		white = !white;
		auto now = gst_clock_get_time(theclock);
		auto base_time = gst_element_get_base_time(app);
		GST_BUFFER_PTS(buffer) = now - base_time;
		//GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 24);

		//timestamp += GST_BUFFER_DURATION(buffer);
		GstFlowReturn ret;
		g_signal_emit_by_name(app, "push-buffer", buffer, &ret);
		gst_buffer_unref(buffer);

		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 24));
	}

	return true;
}

static gboolean
read_data(std::future<GstElement*> f)
{
	auto* app = f.get();
	return _read_data(app);
}


void sendData(GstElement* appSrc) {
	assert(appSrc);

}


GstElement * add_element(GstElement *mixer, GstElement *pipeline) {
	std::this_thread::sleep_for(std::chrono::seconds(10));
	GstElement * appSrc = gst_element_factory_make("appsrc", NULL);
	assert(appSrc);
	/* setup */
	g_object_set(G_OBJECT(appSrc), "caps",
		gst_caps_new_simple("video/x-raw",
			"format", G_TYPE_STRING, "RGB",
			"width", G_TYPE_INT, 384,
			"height", G_TYPE_INT, 288,
			"framerate", GST_TYPE_FRACTION, 0, 1,
			"do-timestamp", G_TYPE_BOOLEAN, true,
			NULL), NULL);
	g_object_set(G_OBJECT(appSrc),
		"stream-type", 0,
		"format", GST_FORMAT_TIME, "is_live", true,
		"min-latency", 0, NULL);
	gst_element_set_state(appSrc, GST_STATE_PLAYING);

	gst_bin_add(GST_BIN(pipeline), appSrc);
	gst_element_link_many(appSrc, mixer, NULL);
	
	return appSrc;
}

int
main(int   argc,
	char *argv[])
{

	int count = 0;
	char** ptr = nullptr;
	gst_init(&count, &ptr);

	/* Initialisation */
	//gst_init(&argc, &argv);

	GMainLoop* loop = g_main_loop_new(NULL, FALSE);

	assert(loop);
	GError* error = nullptr;
	//gst-launch-1.0 -v videomixer name=mixer ! autovideosink sync=false  ksvideosrc ! mixer. videotestsrc pattern=18 ! mixer.
	GstElement *pipeline = gst_parse_launch( 
		"videomixer name=mixer sink_0::xpos=0 sink_1::xpos=800 sink_2::zorder=1  sink_3::ypos=600  sink_4::xpos=800 sink_4::ypos=600 ! autovideosink sync=false \
          videotestsrc pattern=18 ! video/x-raw,format=I420,width=800,height=600 ! mixer. \
          ksvideosrc name=camera ! video/x-raw,format=I420,width=1280,height=720,framerate=25/1 ! mixer. \
          filesrc location=D:/logo.png ! pngdec !  imagefreeze ! mixer.",
		&error);
	assert(error == nullptr);

	
		
	/* Set the pipeline to "playing" state*/
	gst_element_set_state(pipeline, GST_STATE_PLAYING);
//
	
//
//	/*get the clock now.Since we never set the pipeline to PAUSED again, the
//		* clock will not change, even when we add new clock providers later.  */
		theclock = gst_element_get_clock(pipeline);
		assert(theclock);
	pushData = true;
	auto* mixer = gst_bin_get_by_name(GST_BIN(pipeline), "mixer");
	assert(mixer);
//	auto* src = add_element(mixer, pipeline);
	auto first = std::async(std::launch::async, add_element, mixer, pipeline);
    auto fut =   std::async(std::launch::async, read_data, std::move(first));
	/* Iterate */
	g_print("Running...\n");
	g_main_loop_run(loop);
	pushData = false;
	/* Out of the main loop, clean up nicely */
	g_print("Returned, stopping playback\n");
	gst_element_set_state(pipeline, GST_STATE_NULL);

	g_print("Deleting pipeline\n");
	gst_object_unref(GST_OBJECT(pipeline));

	return 0;
}

