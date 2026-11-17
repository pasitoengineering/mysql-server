/* DISTANCE(a, b, metric) for VECTOR columns, which stock MySQL Community lacks. */
#include <math.h>
#include <string.h>
#include <strings.h>
#include <mysql.h>

bool distance_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  if (args->arg_count != 3) {
    strcpy(message, "DISTANCE(a, b, metric) takes 3 arguments");
    return true;
  }
  args->arg_type[0] = STRING_RESULT;
  args->arg_type[1] = STRING_RESULT;
  args->arg_type[2] = STRING_RESULT;
  initid->maybe_null = true;
  initid->decimals = 31;
  return false;
}

void distance_deinit(UDF_INIT *initid) { (void)initid; }

double distance(UDF_INIT *initid, UDF_ARGS *args, unsigned char *is_null,
                unsigned char *error) {
  (void)initid;
  if (!args->args[0] || !args->args[1] || !args->args[2]) {
    *is_null = 1;
    return 0;
  }
  unsigned long la = args->lengths[0], lb = args->lengths[1];
  if (la % 4 != 0 || lb % 4 != 0) {
    *is_null = 1;
    return 0;
  }
  /* Zero-pads the shorter vector rather than rejecting mismatched dimensions. */
  size_t n = (la > lb ? la : lb) / 4;
  const char *pa = args->args[0], *pb = args->args[1];
  double dot = 0, na = 0, nb = 0, sq = 0;
  for (size_t i = 0; i < n; i++) {
    float x = 0, y = 0;
    if (i * 4 < la) memcpy(&x, pa + i * 4, 4);
    if (i * 4 < lb) memcpy(&y, pb + i * 4, 4);
    dot += (double)x * y;
    na += (double)x * x;
    nb += (double)y * y;
    sq += ((double)x - y) * ((double)x - y);
  }
  const char *metric = args->args[2];
  unsigned long lm = args->lengths[2];
  if (lm == 6 && strncasecmp(metric, "COSINE", 6) == 0) {
    /* A zero vector is reported as maximally dissimilar. */
    if (na == 0 || nb == 0) return 1.0;
    return 1.0 - dot / (sqrt(na) * sqrt(nb));
  }
  if (lm == 9 && strncasecmp(metric, "EUCLIDEAN", 9) == 0) {
    return sqrt(sq);
  }
  *error = 1;
  return 0;
}
