#!/usr/bin/env bash
# Make it executable by running following command in bash:
# chmod +x Pi_run_dashboard.sh
# — Activate your venv (if you use one) —
source E:\Perfusion_System\.venv\Scripts\activate
# — Run your Streamlit app —
streamlit run "E:\Perfusion_System\Dashboard\dashboard.py" --server.port 8503
