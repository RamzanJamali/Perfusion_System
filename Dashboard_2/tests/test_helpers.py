# test_helpers.py
import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../')))
from Dashboard_2.dashboard_2 import make_db_filename  # Adjust import path
from unittest.mock import patch
import datetime

"""
def test_make_db_filename():
    # Mock fixed datetime for reproducible testing
    fixed_time = datetime.datetime(2023, 10, 5, 14, 30, 15)

    with patch("dashboard.datetime.datetime") as mock_datetime:
        mock_datetime.now.return_value = fixed_time

        # Call the function
        result = make_db_filename()

    # Verify the result
    assert result == "databases/perfusion_20231005_143015.db"
"""

def test_file_creation(tmp_path):
    # Patch the base directory used in make_db_filename if it uses a BASE_DIR or similar variable
    with patch("dashboard.DB_DIR", tmp_path):
        filename = make_db_filename()
        full_path = tmp_path / filename
        
        # Create dummy file
        full_path.parent.mkdir(parents=True, exist_ok=True)
        full_path.touch()
        
        assert full_path.exists()
        assert "perfusion_" in filename
        assert filename.endswith(".db")