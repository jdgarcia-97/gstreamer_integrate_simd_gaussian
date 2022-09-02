/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2022 maya <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-myfilter
 *
 * FIXME:Describe myfilter here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! myfilter ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstmyfilter.h"
#include <string.h>
#include <stdint.h>
#include <gst/video/video.h>
#include <gst/video/video-enumtypes.h>
#include <gst/video/video-info.h>
#include "SimdLib.h" 
#include <gst/video/video-frame.h>
#include <gst/video/gstvideofilter.h>

//#include <opencv2/opencv.hpp>

GST_DEBUG_CATEGORY_STATIC (gst_my_filter_debug);
#define GST_CAT_DEFAULT gst_my_filter_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT,
  PROP_SIGMA,
  PROP_EPSILON
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

#define gst_my_filter_parent_class parent_class
G_DEFINE_TYPE (GstMyFilter, gst_my_filter, GST_TYPE_ELEMENT);

GST_ELEMENT_REGISTER_DEFINE (my_filter, "my_filter", GST_RANK_NONE,
    GST_TYPE_MYFILTER);

static void gst_my_filter_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_my_filter_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static gboolean gst_my_filter_sink_event (GstPad * pad,
    GstObject * parent, GstEvent * event);
static GstFlowReturn gst_my_filter_chain (GstPad * pad,
    GstObject * parent, GstBuffer * buf);

/* GObject vmethod implementations */

/* initialize the myfilter's class */
static void
gst_my_filter_class_init (GstMyFilterClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_my_filter_set_property;
  gobject_class->get_property = gst_my_filter_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_SIGMA,
      g_param_spec_float("sigma", "Sigma", "Pass in a sigma value",
          -2.00, 20.0, 1.0, G_PARAM_READWRITE)); 

  g_object_class_install_property (gobject_class, PROP_EPSILON, 
      g_param_spec_float("epsilon", "Epsilon", "Pass in an epsilon value",
          0.0001, 10, 0.001, G_PARAM_READWRITE));

  gst_element_class_set_details_simple (gstelement_class,
      "MyFilter",
      "FIXME:Generic",
      "FIXME:Generic Template Element", "jdgarcia97@proton.me");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad callback functions
 * initialize instance structure
 */
static void
gst_my_filter_init (GstMyFilter * filter)
{
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_event_function (filter->sinkpad,
      GST_DEBUG_FUNCPTR (gst_my_filter_sink_event));

  gst_pad_set_chain_function (filter->sinkpad,
      GST_DEBUG_FUNCPTR (gst_my_filter_chain));

  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS (filter->srcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);
 
}

static void
gst_my_filter_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstMyFilter *filter = GST_MYFILTER (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      g_print("Silent argument was changed to %s\n", filter->silent ? "true" : "false");
      break;
    case PROP_SIGMA:
      filter->sigma = g_value_get_float(value);  // Set the sigma value the user passes in.
      break;
    case PROP_EPSILON:
      filter->epsilon = g_value_get_float(value);
      break; 
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_my_filter_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstMyFilter *filter = GST_MYFILTER (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    case PROP_SIGMA:
      g_value_set_float(value, filter->sigma);
      break; 
    case PROP_EPSILON:
      g_value_set_float(value, filter->epsilon);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_my_filter_sink_event (GstPad * pad, GstObject * parent,
    GstEvent * event)
{
  GstMyFilter *filter;
  gboolean ret;

  filter = GST_MYFILTER (parent);

  GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT,
      GST_EVENT_TYPE_NAME (event), event);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
      GstCaps *caps;

      gst_event_parse_caps (event, &caps);
      /* do something with the caps */

      /* and forward */
      ret = gst_pad_event_default (pad, parent, event);
      break;
    }
    default:
      ret = gst_pad_event_default (pad, parent, event);
      break;
  }
  return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_my_filter_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
  GstMyFilter *filter;
  GstVideoFrame frame; 
  GstVideoInfo *vinfo = gst_video_info_new();
  gst_video_info_init(vinfo);

  GstCaps *caps = gst_pad_get_current_caps(pad);
  gst_video_info_from_caps(vinfo, caps); 

  filter = GST_MYFILTER (parent);
  gint width, height; 
  gint stride_y;
  gint stride_u;
  gint stride_v;

   if( gst_video_frame_map(&frame, vinfo, buf, GST_MAP_WRITE) == TRUE){

     uint8_t *pixels_y = GST_VIDEO_FRAME_COMP_DATA( &frame, 0); 
     uint8_t *pixels_u = GST_VIDEO_FRAME_COMP_DATA( &frame, 1);
     uint8_t *pixels_v = GST_VIDEO_FRAME_COMP_DATA( &frame, 2); 

     stride_y = GST_VIDEO_FRAME_PLANE_STRIDE( &frame, 0);
     stride_u = GST_VIDEO_FRAME_PLANE_STRIDE( &frame, 1);
     stride_v = GST_VIDEO_FRAME_PLANE_STRIDE( &frame, 2); 

     width  = GST_VIDEO_INFO_WIDTH( vinfo); 
     height = GST_VIDEO_INFO_HEIGHT( vinfo); 

     gint width_y = GST_VIDEO_FRAME_COMP_WIDTH( &frame, 0);
     gint height_y = GST_VIDEO_FRAME_COMP_HEIGHT( &frame, 0);

     gint width_u = GST_VIDEO_FRAME_COMP_WIDTH( &frame, 1);
     gint height_u = GST_VIDEO_FRAME_COMP_HEIGHT( &frame, 1);

     gint width_v = GST_VIDEO_FRAME_COMP_WIDTH( &frame, 2);
     gint height_v = GST_VIDEO_FRAME_COMP_HEIGHT( &frame, 2); 
     
	 
     /*   g_print("We have a stride_y: %d\n",stride_y );
        g_print("We have a stride_u: %d\n",stride_u );
        g_print("We have a stride_v: %d\n",stride_v );

	g_print("We have a width_y: %d\n",width_y );
	g_print("We have a width_u: %d\n",width_u );
	g_print("We have a width_v: %d\n",width_v );

	g_print("We have a height_y: %d\n",height_y ); 
	g_print("We have a height_u: %d\n",height_u ); 
	g_print("We have a height_v: %d\n",height_v );  */
         
        float sigma = filter->sigma;
        float epsilon = filter->epsilon;	
        



        void *filter_y = SimdGaussianBlurInit( (size_t) width_y ,(size_t) height_y ,(size_t) 1 ,&sigma ,&epsilon);
        void *filter_u = SimdGaussianBlurInit( (size_t) width_u ,(size_t) height_u ,(size_t) 1 ,&sigma ,&epsilon);
        void *filter_v = SimdGaussianBlurInit( (size_t) width_v ,(size_t) height_v ,(size_t) 1 ,&sigma ,&epsilon);

        
	SimdGaussianBlurRun( filter_y,pixels_y ,(size_t) stride_y , pixels_y ,(size_t) stride_y);	  
        SimdGaussianBlurRun( filter_u,pixels_u ,(size_t) stride_u , pixels_u ,(size_t) stride_u);	   
        SimdGaussianBlurRun( filter_v,pixels_v ,(size_t) stride_v , pixels_v ,(size_t) stride_v);


	/*for( h = 0; h < height; ++h){
		for( w = 0; w < width; ++w){
                  //convert RGB pixels to a darker color.
		  guint8 *pixel = pixels + h * stride + w * pixel_stride;
                            
		  memset(pixel, 0, pixel_stride);
		}
	}*/ 
	gst_video_frame_unmap(&frame);
   } 
  /* just push out the incoming buffer without touching it */
  return gst_pad_push (filter->srcpad, buf);
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
myfilter_init (GstPlugin * myfilter)
{
  /* debug category for filtering log messages
   *
   * exchange the string 'Template myfilter' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_my_filter_debug, "myfilter",
      0, "Template myfilter");

  return GST_ELEMENT_REGISTER (my_filter, myfilter);
}

/* PACKAGE: this is usually set by meson depending on some _INIT macro
 * in meson.build and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use meson to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstmyfilter"
#endif

/* gstreamer looks for this structure to register myfilters
 *
 * exchange the string 'Template myfilter' with your myfilter description
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    myfilter,
    "my_filter",
    myfilter_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
