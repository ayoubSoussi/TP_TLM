/*\
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MutekH; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Author: Frédéric Pétrot <Frederic.Petrot@univ-grenoble-alpes.fr>
 *
 * Minimal trap handler just to call the routines
\*/

#include <stdint.h>
#include <address_map.h>
#include <hal.h>

extern void interrupt_handler();

void c_trap_handler(uint32_t cause, uint32_t ip, uint32_t ie)
{
   if ((cause & 0x8000000b) == 0x8000000b)
      interrupt_handler();
   else {
      printf("Unknown trap cause, resuming execution\n");
   }
}
