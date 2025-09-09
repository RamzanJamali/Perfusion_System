@echo off
REM — Activate your venv (if you use one); otherwise skip these two lines —
call E:\Perfusion_System\.venv\Scripts\activate.bat
REM — Run your Streamlit app —
streamlit run "E:\Perfusion_System\save_csv\csv.py" --server.port 8502
pause
