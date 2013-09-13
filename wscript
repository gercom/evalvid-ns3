## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

ddef build(bld):
    module = bld.create_ns3_module('evalvid', ['internet','applications'])
    module.source = [
        'model/evalvid-client.cc',
        'model/evalvid-server.cc',
        'helper/evalvid-client-server-helper.cc',
        ]
    headers = bld (features=['ns3header'])
    headers.module = 'evalvid'
    headers.source = [
        'model/evalvid-client.h',
        'model/evalvid-server.h',
        'helper/evalvid-client-server-helper.h',
        ]
    if bld.env.ENABLE_EXAMPLES:
      bld.add_subdirs('examples')    