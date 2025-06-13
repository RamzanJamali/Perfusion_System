# test_streamlit_app.py
from playwright.sync_api import Page, expect


def test_pressure_number_input(page: Page):
    # Launch Streamlit app (run app locally first)
    page.goto("http://localhost:8501")
    
    # Locate the number input by accessible name
    pressure_input = page.get_by_role("spinbutton", name="Pressure (mmHg)")
    
    # 1. Test initial value
    expect(pressure_input).to_have_value("1.00")
    
    # 2. Test valid input
    pressure_input.fill("75.5")
    expect(pressure_input).to_have_value("75.5")
    
    # 3. Test min/max clamping
    pressure_input.fill("100.0")  # Above max
    pressure_input.blur()  # Trigger validation
    expect(pressure_input).to_have_value("100.00")  # Clamped to max
    
    pressure_input.fill("0.0")  # Below min
    pressure_input.blur()
    expect(pressure_input).to_have_value("0.00")  # Clamped to min

    # 4. Test step navigation
    pressure_input.fill("1.00")
    pressure_input.press("ArrowUp")  # Simulate ↑ key press
    expect(pressure_input).to_have_value("2.00")  # Increased by 1.0

    pressure_input.press("ArrowDown")  # Simulate ↓ key press
    expect(pressure_input).to_have_value("1.00")


def test_flow_number_input(page: Page):
    # Launch Streamlit app (run app locally first)
    page.goto("http://localhost:8501")
    
    # Locate the number input by accessible name
    flow_input = page.get_by_role("spinbutton", name="Flow Rate (ml/day) -> min value 1.7")

    # 1. Test initial value
    expect(flow_input).to_have_value("1.70")

    # 2. Test valid input
    flow_input.fill("75.5")
    expect(flow_input).to_have_value("75.5")

    # 3. Test min/max clamping
    flow_input.fill("100.0")  # Above max
    flow_input.blur()  # Trigger validation
    expect(flow_input).to_have_value("100.00")  # Clamped to max

    flow_input.fill("0.0")  # Below min
    flow_input.blur()
    expect(flow_input).to_have_value("0.0")  # Clamped to min

    # 4. Test step navigation
    flow_input.fill("1.70")
    flow_input.press("ArrowUp")  # Simulate ↑ key press
    expect(flow_input).to_have_value("1.71")  # Increased by 1.0

    flow_input.press("ArrowDown")  # Simulate ↓ key press
    expect(flow_input).to_have_value("1.70")

