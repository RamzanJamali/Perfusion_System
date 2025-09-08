#!/usr/bin/env bash
# Make it executable by running following command in bash:
# chmod +x Pi_run_csv.sh
# — Activate your venv (if you use one) —
source /home/AG-Lang/Downloads/Perfusion_System/venv/bin/activate
# — Run your Streamlit app —
streamlit run "/home/AG-Lang/Downloads/Perfusion_System/save_csv/csv.py" --server.port 8502
