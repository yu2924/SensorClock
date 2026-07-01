This folder contains the tools for generating JSON tables from TSV files.
Once generated, the JSON tables should be copied to the "firmware/assets" folder for use.
Currently, it is used to generate the following tables:
- list of representative time zones
- list of 3rd party license notices

Usage:

python tsv_to_json.py timezones50.tsv
python tsv_to_json.py table_of_licenses.tsv
