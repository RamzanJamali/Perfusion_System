import sqlite3
import pandas as pd
import streamlit as st
import tempfile
from io import StringIO


st.set_page_config(
    page_title="Perfusion Data Viewer",
    page_icon="📊",
    layout="wide",
)


# ---- Functions ----
def get_connection(db_path: str) -> sqlite3.Connection:
    """
    Establish a connection to the SQLite database.
    """
    conn = sqlite3.connect(db_path)
    return conn


def fetch_all_readings(conn: sqlite3.Connection) -> pd.DataFrame:
    """
    Fetch all rows from the specified table into a pandas DataFrame.
    """
    query = f"SELECT * FROM sensor_readings"
    df = pd.read_sql_query(query, conn)
    return df


def save_dataframe_to_csv(df: pd.DataFrame) -> str:
    """
    Convert a DataFrame to a CSV string for download.
    """
    csv_buffer = StringIO()
    df.to_csv(csv_buffer, index=False)
    return csv_buffer.getvalue()


# ---- Streamlit App ----
st.title("📊 Save Perfusion Data to CSV File")

# ---- Database File Selection ----
uploaded_db = st.file_uploader(
    label="Choose a SQLite database file", 
    type=["db", "sqlite", "sqlite3"]
)

if uploaded_db is not None:
    # Save uploaded file to a temporary location
    tmp_file = tempfile.NamedTemporaryFile(delete=False, suffix=".db")
    tmp_file.write(uploaded_db.read())
    tmp_file.flush()
    db_path = tmp_file.name

    # Fetch and display readings
    if st.button("🔄 Load All Readings"):
        with st.spinner("Fetching data..."):
            conn = get_connection(db_path)
            df = fetch_all_readings(conn)
            conn.close()

            if df.empty:
                st.warning("No readings found in the database.")
            else:
                st.success(f"Loaded {len(df)} records from sensor_readings.")
                

                # Offer CSV download
                csv_buffer = save_dataframe_to_csv(df)
                st.download_button(
                    label="📥 Download CSV",
                    data=csv_buffer,
                    file_name=f"sensor_readings_export.csv",
                    mime="text/csv"
                )
                st.dataframe(df)
else:
    st.info("Please upload a SQLite database file to begin.")

# Footer
st.markdown("---")
st.caption("Built with ❤️ using Streamlit and SQLite")
