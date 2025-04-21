#!/bin/bash
fswatch -0 -o ./src ./tests ./libs CMakeLists.txt | ./compile.sh