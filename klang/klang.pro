TEMPLATE=subdirs
CONFIG +=ordered
SUBDIRS =base k \
         test_string \
         test_tokenizer

test_string.file    = test_string.pro
test_tokenizer.file = test_tokenizer.pro
base.file           = base.pro
k.file              = k.pro

