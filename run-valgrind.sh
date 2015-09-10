rm -rf ./.hide
#valgrind --tool=helgrind ./test.py
valgrind --tool=helgrind ./bin/hide_test
