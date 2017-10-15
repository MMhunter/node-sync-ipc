{
  "targets": [
    {
      "target_name": "server",
      "sources": ["src/server.cpp" ],
      "include_dirs" : [
            "<!(node -e \"require('nan')\")"
        ]
    },
    {
      "target_name": "client",
      "sources": [ "src/client.cpp" ],
      "include_dirs" : [
            "<!(node -e \"require('nan')\")"
        ]
    }
  ]
}