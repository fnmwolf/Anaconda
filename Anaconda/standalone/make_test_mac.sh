export PYTHONPATH="$PYTHONPATH:.."
rm -rf make_test_mac
mkdir make_test_mac
python -O make.py ../test/InvertInc.ccn make_test_mac/InvertInc.zip