#!/bin/sh

NAME=coprthr_sdk-1.0-ALPHA

tar -zcvf outlaw.$NAME.`date +%Y%m%d.%H%M`.tgz ./$NAME/COPYING ./$NAME/COPYING.LESSER ./$NAME/Makefile ./$NAME/libs/libstdcl ./$NAME/tools/clld ./$NAME/tools/cltrace ./$NAME/include


