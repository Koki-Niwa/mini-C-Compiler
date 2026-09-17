#!/bin/bash
# 测试必须自洽：不管外面 shell 有没有 export CALC_DEBUG，都不能受影响
export CALC_DEBUG=0

assert() {
  expected="$1"
  input="$2"

  ./main "$input" > tmp.s || { echo "compile error: ./main '$input'"; exit 1; }
  cc -o tmp tmp.s || { echo "assemble/link error: ./main '$input'"; exit 1; }
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 10-10
assert 42 30+10+2
assert 0 1+2-3+4-4
assert 0 '1 - 1'

echo OK
