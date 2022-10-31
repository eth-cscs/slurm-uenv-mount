#!/bin/bash

exec /entrypoint.sh sh -c "su  testuser -P  -s /bin/bash -l"
