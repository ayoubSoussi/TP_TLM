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
 * Boot:
 * Just what is strictly necessary to do some testing.
 * The vector values are platform specific, I'm using those of rocket.
\*/
	.extern _stack_top
	.globl _start

	.text
_boot:
	# we assume a single address for all traps
	la      t0, __trap_handler
	csrw    mtvec, t0

	# we also assume there is a single hart, so setting the
	# stack is easy! Otherwise, we can do as in mutek, by assigning
        # a small stack based on the hart number (not bulletproof, though)

	la      sp, _stack_top
        jal     x1, main

_boot_end:
	wfi /* since as opposed to the µBlz, it exists in risc-v */
	j _boot_end
