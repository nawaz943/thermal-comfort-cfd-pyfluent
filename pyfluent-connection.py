import ansys.fluent.core as pyfluent

# This looks for that .txt file and connects to your open session
session = pyfluent.connect_to_fluent(server_info_file_name=r"D:\ansys-files2\temp\temp-dir\room\server_info-59024.txt")

# Let's confirm it works by printing the name of your mesh/case file
print("Successfully connected!")

# use this to confirm if pyfluent reads case with data (cfd results)
# print("Current Case Name:", session.settings.file.read_case_data())

# In 2026, we can access zone names via the cell_zone_conditions settings
fluid_zones = session.settings.setup.cell_zone_conditions.fluid.get_object_names()
solid_zones = session.settings.setup.cell_zone_conditions.solid.get_object_names()

print(f"Fluid Zones: {fluid_zones}")
print(f"Solid Zones: {solid_zones}")
