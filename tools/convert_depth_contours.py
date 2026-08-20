"""One-off conversion: clip the national depth-contour shapefile to a lake's
bounding box, reproject EUREF-FIN TM35FIN (EPSG:3067) to WGS84, simplify, and
write GeoJSON small enough for the ESP32's LittleFS partition (128 KiB).

Usage: python tools/convert_depth_contours.py
"""
import json

import shapefile
from pyproj import Transformer
from shapely.geometry import LineString, box, mapping

SOURCE_SHAPEFILE = "tools/gis-source/Syvyyskayra.shp"
OUTPUT_GEOJSON = "data/tuusulanjarvi.geojson"

# Degrees; ~1e-5 deg is roughly 1 m at this latitude
SIMPLIFY_TOLERANCE_DEGREES = 0.00003
COORDINATE_DECIMALS = 6

# Bounding box supplied as DMS, converted to decimal degrees (WGS84)
MIN_LAT = 60 + 24 / 60 + 30.3 / 3600
MIN_LON = 25 + 1 / 60 + 10.2 / 3600
MAX_LAT = 60 + 27 / 60 + 59.9 / 3600
MAX_LON = 25 + 4 / 60 + 57.7 / 3600

to_wgs84 = Transformer.from_crs("EPSG:3067", "EPSG:4326", always_xy=True)
to_tm35fin = Transformer.from_crs("EPSG:4326", "EPSG:3067", always_xy=True)

# Pre-filter bbox in the source projection to skip most of the 90k nationwide records
src_min_x, src_min_y = to_tm35fin.transform(MIN_LON, MIN_LAT)
src_max_x, src_max_y = to_tm35fin.transform(MAX_LON, MAX_LAT)

clip_box = box(MIN_LON, MIN_LAT, MAX_LON, MAX_LAT)

reader = shapefile.Reader(SOURCE_SHAPEFILE)
features = []

for shape_record in reader.iterShapeRecords():
    shape = shape_record.shape
    bx_min, by_min, bx_max, by_max = shape.bbox
    if bx_max < src_min_x or bx_min > src_max_x or by_max < src_min_y or by_min > src_max_y:
        continue

    lonlat_points = [to_wgs84.transform(x, y) for x, y in shape.points]
    line = LineString(lonlat_points)
    clipped = line.intersection(clip_box)
    if clipped.is_empty:
        continue

    depth_label = shape_record.record["Syvyyskayr"].replace(",", ".")
    geometries = [clipped] if clipped.geom_type == "LineString" else list(clipped.geoms)
    for geometry in geometries:
        if geometry.is_empty or geometry.geom_type != "LineString":
            continue
        simplified = geometry.simplify(SIMPLIFY_TOLERANCE_DEGREES, preserve_topology=False)
        if simplified.is_empty or len(simplified.coords) < 2:
            continue
        rounded_coords = [
            (round(lon, COORDINATE_DECIMALS), round(lat, COORDINATE_DECIMALS))
            for lon, lat in simplified.coords
        ]
        features.append({
            "type": "Feature",
            "properties": {"depth": float(depth_label)},
            "geometry": {"type": "LineString", "coordinates": rounded_coords},
        })

geojson = {"type": "FeatureCollection", "features": features}
with open(OUTPUT_GEOJSON, "w", encoding="utf-8") as handle:
    json.dump(geojson, handle, ensure_ascii=False, separators=(",", ":"))

print(f"Wrote {len(features)} contour features to {OUTPUT_GEOJSON}")
