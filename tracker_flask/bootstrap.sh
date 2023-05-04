#!/bin/sh
export FLASK_APP=./trackerAPI/app.py
# pipenv run flask --debug run -h 0.0.0.0./bootstrap.sh 
source $(pipenv --venv)/bin/activate
flask run -h 0.0.0.0