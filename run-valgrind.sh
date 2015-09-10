Fail() {
	echo "$@" >&2
	exit 1
}

[ $# -eq 1 ] || Fail "Usage: $0 <executable>"
rm -rf ./.hide
valgrind --suppressions=etc/valgrind-python.supp --tool=helgrind $1
