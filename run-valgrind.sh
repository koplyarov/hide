Fail() {
	echo "$@" >&2
	exit 1
}

[ $# -eq 1 ] || Fail "Usage: $0 <executable>"
rm -rf ./.hide
valgrind --tool=helgrind $1
