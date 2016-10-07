#!/bin/bash

for dj in /Users/atuladhar/Google\ Drive/USF/Fall\ 2016/Compilers/Assignments/Assignment1/examples/*.dj; do
    #./dj-lex "$dj"
    ./lexer/lexer "$dj"
done