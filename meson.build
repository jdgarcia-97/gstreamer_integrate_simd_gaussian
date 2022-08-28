project('gst-template', 'c', version : '1.19.0.1', license : 'LGPL')

plugins_install_dir = join_paths(get_option('libdir'), 'gstreamer-1.0')

cc = meson.get_compiler('c')

libs_you_need_to_link = ['Simd']
deps = []

foreach lib_name : libs_you_need_to_link
    deps += cc.find_library(lib_name, dirs : ['/usr/local/lib/'])
endforeach

gst_version = meson.project_version()

api_version = '1.0'

gst_dep = dependency('gstreamer-1.0', version : '>=1.19',
    required : true, fallback : ['gstreamer', 'gst_dep'])
gstbase_dep = dependency('gstreamer-base-1.0', version : '>=1.19',
  fallback : ['gstreamer', 'gst_base_dep'])


subdir('gst-app')
subdir('gst-plugin')
