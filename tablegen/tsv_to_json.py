from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path


def normalize_tsv_value(value: str | None) -> str:
    if value is None:
        return ""
    return value.replace("\\n", "\n")


def convert_tsv_to_json(input_path: Path, output_path: Path) -> None:
    with input_path.open("r", encoding="utf-8-sig", newline="") as input_file:
        reader = csv.DictReader(input_file, delimiter="\t")
        if reader.fieldnames is None:
            raise ValueError("TSV header is missing.")

        rows = []
        for row in reader:
            rows.append({key: normalize_tsv_value(value) for key, value in row.items()})

    with output_path.open("w", encoding="utf-8", newline="") as output_file:
        json.dump(rows, output_file, ensure_ascii=False, indent="\t")
        output_file.write("\n")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Convert a tab-separated values file into a JSON array of objects."
    )
    parser.add_argument("input_tsv", type=Path, help="Path to the input TSV file.")
    parser.add_argument(
        "output_json",
        type=Path,
        nargs="?",
        help="Path to the output JSON file. Defaults to the input name with a .json extension.",
    )
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    input_path = args.input_tsv
    output_path = args.output_json or input_path.with_suffix(".json")

    if not input_path.is_file():
        parser.error(f"Input TSV file was not found: {input_path}")

    convert_tsv_to_json(input_path, output_path)
    print(f"Wrote {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())