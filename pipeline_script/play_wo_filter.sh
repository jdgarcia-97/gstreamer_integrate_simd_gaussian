#!/bin/sh

time gst-launch-1.0 filesrc location="./sample_1280x720_surfing_with_audio.ts" ! tsdemux ! queue ! mpegvideoparse ! mpeg2dec ! videoconvert ! queue ! autovideosink


