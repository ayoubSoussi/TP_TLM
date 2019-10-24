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
 * Trap handling for integer registers only (for now):
 * Context saving : we call a c handler, so, as the compiler preserves the
 * callee-saved registers, we have to save the caller-saved ones.
 * This is sorted alphabetically, just in case you wonder
 * gp and tp stay untouched
\*/
	.global  __trap_handler
	.text
	#mtvec wants an address that is a multiple of 4
	.align 2 # means 1 << 2, yeah
__trap_handler:
	addi  sp, sp, -16*4
	sw    ra,  0*4(sp)
	sw    a0,  1*4(sp)
	sw    a1,  2*4(sp)
	sw    a2,  3*4(sp)
	sw    a3,  5*4(sp)
	sw    a4,  5*4(sp)
	sw    a5,  6*4(sp)
	sw    a6,  7*4(sp)
	sw    a7,  8*4(sp)
	sw    t0,  9*4(sp)
	sw    t1, 10*4(sp)
	sw    t2, 11*4(sp)
	sw    t3, 12*4(sp)
	sw    t4, 13*4(sp)
	sw    t5, 14*4(sp)
	sw    t6, 15*4(sp)
        csrr  a0, mcause
        csrr  a1, mip
        csrr  a2, mie
	jal   ra, c_trap_handler
	lw    ra,  0*4(sp)
	lw    a0,  1*4(sp)
	lw    a1,  2*4(sp)
	lw    a2,  3*4(sp)
	lw    a3,  5*4(sp)
	lw    a4,  5*4(sp)
	lw    a5,  6*4(sp)
	lw    a6,  7*4(sp)
	lw    a7,  8*4(sp)
	lw    t0,  9*4(sp)
	lw    t1, 10*4(sp)
	lw    t2, 11*4(sp)
	lw    t3, 12*4(sp)
	lw    t4, 13*4(sp)
	lw    t5, 14*4(sp)
	lw    t6, 15*4(sp)
	addi  sp, sp, 16*4
	mret
