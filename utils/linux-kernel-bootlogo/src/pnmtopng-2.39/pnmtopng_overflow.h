/* Wild guess/reimplementation/adaptation of Alan Cox's (presumed) overflow-
 * protection function (seen used in http://security.debian.org/pool/updates/
 * main/n/netpbm-free/netpbm-free_9.20-8.4.diff.gz).  Greg Roelofs, 20051112 */

static void grr_overflow2(int mult1, int mult2)
{
  int product = mult1 * mult2;

  if (mult1 <= 0 || mult2 <= 0 || product/mult1 < mult2) {
    pm_error ("dimensions would overflow memory allocation");
  }
}
