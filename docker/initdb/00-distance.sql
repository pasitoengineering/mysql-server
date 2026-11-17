-- DISTANCE(a, b, metric) for VECTOR columns, as a server-wide loadable function.
CREATE FUNCTION distance RETURNS REAL SONAME 'distance_udf.so';
