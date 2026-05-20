import pandas as pd

csv_path = r"D:\weather-analysis\scripts\set-pyfluent\exported_data\calculated_SET_results.csv"

print("Reading CSV for high-speed spatial sorting...")
df = pd.read_csv(csv_path)

print("Sorting 55 million rows by X-coordinate...")
df = df.sort_values(by='x-coordinate', ascending=True)

print("Saving optimized dataset...")
df.to_csv(csv_path, index=False)
print("Done! File is ready for high-speed UDF streaming.")