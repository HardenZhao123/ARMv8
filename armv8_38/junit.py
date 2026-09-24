#!/usr/bin/env python3

import json
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

TEST_TYPES = ["emulator", "assembler"]

testsuite_dir = Path(sys.argv[1])
test_cases_dir = testsuite_dir / "test/test_cases"
actual_results_dir = testsuite_dir / "test/actual_results"


def get_actual_results(test: str, suffix: str) -> Path:
    return actual_results_dir / (testsuite_dir / Path(test)).relative_to(
        test_cases_dir
    ).with_suffix(suffix)


with open(actual_results_dir / "results.json") as f:
    results = json.load(f)

testsuites = ET.Element("testsuites")
for test_type in TEST_TYPES:
    summary = results[f"{test_type}_summary"]
    testsuite = ET.SubElement(
        testsuites,
        "testsuite",
        name=test_type,
        tests=str(summary["total"]),
        failures=str(summary["incorrect"]),
        errors=str(summary["failed"]),
    )
    for test, result in results[test_type].items():
        testcase = ET.SubElement(
            testsuite,
            "testcase",
            classname=test_type,
            name=test,
        )
        with open(get_actual_results(test, ".json")) as f:
            info = json.load(f)
        message = "\n".join(info[test_type]["log"])
        if result == "INCORRECT":
            ET.SubElement(testcase, "failure").text = message
        elif result == "FAILED":
            ET.SubElement(testcase, "error").text = message
        if test_type == "emulator":
            with open(get_actual_results(test, ".out")) as f:
                ET.SubElement(testcase, "system-out").text = f.read()

print(ET.tostring(testsuites, encoding="unicode", xml_declaration=True))
