plugin_c_args = ['-DHAVE_CONFIG_H']

cc = meson.get_compiler('c')


cdata = configuration_data()

cdata.set_quoted('PACKAGE_VERSION', gst_version)
cdata.set_quoted('PACKAGE', 'gst-template-plugin')
cdata.set_quoted('GST_LICENSE', 'LGPL')
cdata.set_quoted('GST_API_VERSION', api_version)
cdata.set_quoted('GST_PACKAGE_NAME', 'GStreamer template Plug-ins')
cdata.set_quoted('GST_PACKAGE_ORIGIN', 'https://gstreamer.freedesktop.org')

configure_file(output : 'config.h', configuration : cdata)

gstaudio_dep = dependency('gstreamer-audio-1.0', fallback: ['gst-plugins-base', 'audio_dep'])
gstvideo_dep = dependency('gstreamer-video-1.0')
my_inc = include_directories('/usr/local/include/Simd')
simd_lib_dir = '/usr/local/lib'
simd_lib = 'Simd'
simd_dep = declare_dependency(
  link_args : ['-L' + simd_lib_dir, '-l' + simd_lib, '-lstdc++'],
  include_directories : my_inc
  )

# my include directories.

# Plugin 1
plugin_sources = [
  'src/gstplugin.c'
  ]

gstpluginexample = library('gstplugin',
  plugin_sources,
  c_args: plugin_c_args,
  dependencies : [gst_dep],
  install : true,
  install_dir : plugins_install_dir,
)

# Plugin 2 (audio filter example)
audiofilter_sources = [
  'src/gstaudiofilter.c',
  ]

gstaudiofilterexample = library('gstaudiofilterexample',
  audiofilter_sources,
  c_args: plugin_c_args,
  dependencies : [gst_dep, gstaudio_dep],
  install : true,
  install_dir : plugins_install_dir,
)

# The myfilter Plugin
 gstmyfilter_sources = [
  'src/gstmyfilter.c',
  ]

gstmyfilterexample = library('gstmyfilter',
  gstmyfilter_sources,
  c_args: plugin_c_args,
  dependencies : [gst_dep, gstbase_dep, gstvideo_dep, simd_dep],
  include_directories: [my_inc],
  install : true,
  install_dir : plugins_install_dir,
)
