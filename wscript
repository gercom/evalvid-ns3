## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('evalvid', ['internet','applications','mobility'])
    module.source = [
        'model/evalvid-client.cc',
        'model/evalvid-server.cc',
        'model/video-type-tag.cc',
        'helper/evalvid-client-server-helper.cc',
        ]
        
    headers = bld (features=['ns3header'])
    headers.module = 'evalvid'
    headers.source = [
        'model/evalvid-client.h',
        'model/evalvid-server.h',
        'helper/evalvid-client-server-helper.h',
        'model/video-type-tag.h',
        ]
        
    if (bld.env['ENABLE_EXAMPLES']):
      bld.recurse('examples')
