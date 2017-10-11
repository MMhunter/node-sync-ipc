{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "hello.cpp" ],
      "include_dirs" : [
            "<!(node -e \"require('nan')\")"
        ]
    }
  ]
}