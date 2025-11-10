#!/bin/sh

mkdir -p /digital_signature/build/${DIGITAL}

cd /digital_signature/build/${DIGITAL}

cmake /digital_signature
cmake --build . --parallel

/digital_signature/build/${DIGITAL}/digital_signature