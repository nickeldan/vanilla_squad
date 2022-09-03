import pathlib
import re
import subprocess
import sys
import tempfile
import time
import typing

import pytest

THIS_DIR = pathlib.Path(__file__).parent
FORMAT_EXECUTABLE = str(THIS_DIR / "format")
HEXDUMP_EXECUTABLE = str(THIS_DIR / "hexdump")

DATE_PATTERN = re.compile(r"[A-Z][a-z]{2} [A-Z][a-z]{2} [01 ]\d \d{2}:\d{2}:\d{2} \d{4}$")
HEXDUMP_HEADER_PATTERN = re.compile(r"stdin \((\d+) bytes?\):$")
HEXDUMP_PATTERN = re.compile(r"\t([a-f0-9]{4})\t((?:[a-f0-9]{2} )+) *\t(.+)")
HEXDUMP_ELLIPSIS_PATTERN = re.compile(r"\t\.\.\. \((\d+) more bytes?\)$")
HEXDUMP_LINE_LENGTH = 16

PRINTABLE_RANGE = range(ord(" "), ord("~") + 1)

FORMAT_OUTPUT_PAIRS = (
    ("", ""),
    ("Hello", "Hello"),
    ("%M", "Check"),
    ("%L", "INFO"),
    ("%_", " " * 4),
    ("%F", "test_format.c"),
    ("%f", "main"),
    ("%%", "%"),
    ("%x%M%x", "0-1Check1-2"),
)


def encode_data(data: bytes) -> typing.Iterator[str]:
    for n in data:
        if n in PRINTABLE_RANGE:
            yield chr(n)
        else:
            yield "."


@pytest.mark.parametrize("format_str,output", FORMAT_OUTPUT_PAIRS)
def test_simple_format(format_str: str, output: str):
    p = subprocess.run([FORMAT_EXECUTABLE, format_str], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert p.stdout == output


def test_format_pid():
    p = subprocess.Popen([FORMAT_EXECUTABLE, "%p"], stdout=subprocess.PIPE, encoding="utf-8")
    stdout, _ = p.communicate()
    assert p.returncode == 0
    assert stdout == str(p.pid)


@pytest.mark.skipif("not sys.platform.startswith('linux')", reason="Feature only supported on Linux")
def test_format_tid():
    p = subprocess.Popen([FORMAT_EXECUTABLE, "%T"], stdout=subprocess.PIPE, encoding="utf-8")
    stdout, _ = p.communicate()
    assert p.returncode == 0
    assert stdout == str(p.pid)


def test_format_epoch_time():
    p = subprocess.run([FORMAT_EXECUTABLE, "%u"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert abs(time.time() - int(p.stdout)) < 2


def test_format_pretty_time():
    p = subprocess.run([FORMAT_EXECUTABLE, "%t"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert DATE_PATTERN.match(p.stdout)


def test_format_hour():
    p = subprocess.run([FORMAT_EXECUTABLE, "%h"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert int(p.stdout) == time.localtime().tm_hour


def test_format_minute():
    p = subprocess.run([FORMAT_EXECUTABLE, "%m"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert int(p.stdout) == time.localtime().tm_min


def test_format_second():
    p = subprocess.run([FORMAT_EXECUTABLE, "%s"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert abs(time.localtime().tm_sec - int(p.stdout)) < 2


def test_format_line_number():
    p = subprocess.run([FORMAT_EXECUTABLE, "%l\n"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    lines = p.stdout.split("\n")
    assert int(lines[1]) == int(lines[0]) + 2


def test_format_bad():
    p = subprocess.run([FORMAT_EXECUTABLE, "%v"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 2


def data_samples() -> typing.Iterator[bytes]:
    yield pytest.param(b"", id='b""')
    yield pytest.param(b"a", id='b"a"')
    yield pytest.param(b"hello", id='b"hello"')

    chunk = bytes(range(256))
    yield pytest.param(chunk, id="chunk")

    yield pytest.param(chunk * 16, id="big chunk")


@pytest.mark.parametrize("data", data_samples())
def test_hexdump(data):
    with tempfile.TemporaryFile() as f:
        f.write(data)
        f.seek(0)
        p = subprocess.run(HEXDUMP_EXECUTABLE, stdin=f, stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0

    lines = p.stdout.split("\n")
    match = HEXDUMP_HEADER_PATTERN.match(lines[0])
    assert match is not None
    assert int(match.group(1)) == len(data)
    lines = lines[1:-1]

    out_data = b""
    extra_length = 0

    if len(lines) > 0:
        match = HEXDUMP_ELLIPSIS_PATTERN.match(lines[-1])
        if match is not None:
            extra_length = int(match.group(1))
            lines = lines[:-1]

    for k, line in enumerate(lines):
        match = HEXDUMP_PATTERN.match(line)
        assert match is not None
        assert int(match.group(1), 16) == HEXDUMP_LINE_LENGTH * k
        line_data = bytes.fromhex(match.group(2).replace(" ", ""))
        line_ascii = match.group(3)
        line_encode = "".join(encode_data(line_data))
        assert line_encode == line_ascii
        out_data += line_data

    assert len(out_data) + extra_length == len(data)
    if extra_length > 0:
        assert out_data == data[:-extra_length]
    else:
        assert out_data == data
