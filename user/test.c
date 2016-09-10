// Generation du test.c parsé :
// cpp -traditional-cpp -C -P -ftabstop=8 -DTELECOM_TST -DCONS_READ_CHAR -DWITH_MSG test.c test.i
// sed -i '/./,/^$/!d' test.i
// cat test.h test.i

//XXX Assurer que l'oubli d'une option fait planter la compilation
//XXX Verifier l'absence de caracteres non ASCII

/*******************************************************************************
 * Gestion de liste d'arguments de taille variable (printf)
 ******************************************************************************/
typedef void *__gnuc_va_list;
typedef __gnuc_va_list va_list;
#define va_arg(AP, TYPE)                                                \
 (AP = (__gnuc_va_list) ((char *) (AP) + __va_rounded_size (TYPE)),     \
  *((TYPE *) (void *) ((char *) (AP) - __va_rounded_size (TYPE))))
#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))
#define va_start(AP, LASTARG)                                           \
 (AP = ((__gnuc_va_list) __builtin_next_arg (LASTARG)))
#define va_end(AP)      ((void)0)

/*******************************************************************************
 * Printf macros
 ******************************************************************************/
#define PRINTF_LEFT_JUSTIFY 1
#define PRINTF_SHOW_SIGN 2
#define PRINTF_SPACE_PLUS 4
#define PRINTF_ALTERNATE 8
#define PRINTF_PAD0 16
#define PRINTF_CAPITAL_X 32

#define PRINTF_BUF_LEN 512

/*******************************************************************************
 * Assert : check a condition or fail
 ******************************************************************************/
#define __STRING(x) #x

#define assert(cond) \
((void)((cond) ? 0 : assert_failed(__STRING(cond), __FILE__, __LINE__)))

#define DUMMY_VAL 78

#define TSC_SHIFT 8

#define FREQ_PREC 50

#define NBSEMS 10000

#define TRUE 1
#define FALSE 0

#define NR_PHILO 5
/*
 * Tests du projet systeme
 *
 * Ce fichier contient du code qui ne depend que des appels systemes
 * ci-dessous. Il n'inclut aucun fichier ".h".
 *
 * Il est possible de placer ce fichier dans le repertoire user pour faire
 * tourner les tests au niveau utilisateur ou, si l'implantation du mode
 * utilisateur ne fonctionne pas, dans le repertoire kernel pour faire
 * tourner les tests au niveau superviseur.
 * Les tests sont separes en 20 fonctions qui testent differentes parties du
 * projet.
 * Aucune modification ne doit etre apportee a ce fichier pour la soutenance.
 *
 * Il existe deux manieres d'appeler les tests :
 *
 * int test_proc(void *arg) :
 * a demarrer comme un processus avec une priorite de 128. Ce processus
 * attend que l'utilisateur saisisse des numeros de tests a executer.
 *
 * int test_run(int n) :
 * a appeler dans un processus de priorite 128 avec en parametre un numero de
 * test a executer.
 *
 * La fonction test_run() vous permet d'appeler facilement un test mais en
 * soutenance il est preferable d'executer test_proc().
 */

// Prototype des appels systeme de la spec
int chprio(int pid, int newprio);
void cons_write(const char *str, unsigned long size);
unsigned long cons_read(char *string, unsigned long length);
void cons_echo(int on);
void exit(int retval);
int getpid(void);
int getprio(int pid);
int kill(int pid);
int scount(int sem);
int screate(short count);
int sdelete(int sem);
int signal(int sem);
int signaln(int sem, short count);
int sreset(int sem, short count);
int try_wait(int sem);
int wait(int sem);
void clock_settings(unsigned long *quartz, unsigned long *ticks);
unsigned long current_clock(void);
void wait_clock(unsigned long wakeup);
int start(int (*ptfunc)(void *), unsigned long ssize, int prio, const char *name, void *arg);
int waitpid(int pid, int *retval);

/*
 * Pour la soutenance, devrait afficher la liste des processus actifs, des
 * semaphores utilises et toute autre info utile sur le noyau.
 */
void sys_info(void);

static int
strcmp(const char *str1, const char *str2)
{
	while (*str1 == *str2) {
		if (*str1 == 0)
			return 0;
		str1++;
		str2++;
	}
	return *str1 - *str2;
}

static unsigned long
strlen(const char *s)
{
	unsigned long l = 0;
	while (*s++) l++;
	return l;
}

static void
cons_puts(const char *s)
{
	cons_write(s, strlen(s));
}

/*******************************************************************************
static int _printf( char *s, unsigned long n, const char *format, va_list ap )
    La grosse fonction du module. Une version interne du printf, qui imprime
soit sur la sortie standard, soit dans un buffer.
Si s == 0 : on imprime sur la sortie standard.
Si s != 0 : on imprime dans la chaine s sur une taille maximale n.
*******************************************************************************/
struct printf_st {
	/* Flags obtenus a l'interpretation de la chaine de format. */
	int flags;
	int width;
	int precision;
	char modifier;
	int count;

	/* Ce buffer permet de contenir le texte correspondant a l'affichage
	   d'un nombre. */
	char buffer_nombre[20];

	/* Buffer pour l'ecriture a la console. */
	char buffer_ecr[PRINTF_BUF_LEN];
	int pos_ecr;

	char *str;
	unsigned long strl;
};

static void
print(struct printf_st *pf, char c)
{
	while (1) {
		if (pf->str == 0) {
			/* Cas de l'ecriture sur un fichier. */
			if (c == 0)
				return;
		
			if (pf->pos_ecr < PRINTF_BUF_LEN - 1) {
				pf->count++;
				pf->buffer_ecr[pf->pos_ecr++] = c;
			} else {
				pf->buffer_ecr[PRINTF_BUF_LEN - 1] = 0;
				cons_puts(pf->buffer_ecr);
				pf->buffer_ecr[0] = c;
				pf->pos_ecr = 1;
			}
		} else {
			/* Cas de l'ecriture dans un buffer. */
			if ((c != 0) && (pf->strl != 0)) {
				pf->count++;
				*pf->str++ = c;
				pf->strl--;
			}
		}
		if (c != '\n') return;
		c = '\r';
	}
}

/****************************************************************************
 * Pour afficher les "%s".
 ***************************************************************************/
static void
print_string(struct printf_st *pf, char *s) {
	int size = 0;
	char *ptr = s;

	/* Calcule la taille de la partie de la chaine a afficher. */
	if (pf->precision >= 0)
		while ((size < pf->precision) && (*ptr++ != 0))
			size++;
	else
		/* Pas besoin d'aller trop loin dans le calcul de la taille de la
			partie a afficher. Si la valeur est superieure a width, elle ne nous
			sert a rien, bien qu'on va tout afficher. */
		while ((size < pf->width) && (*ptr++ != 0))
			size++;

	if (!(pf->flags & PRINTF_LEFT_JUSTIFY))
		while (pf->width-- > size)
			print(pf, ' ');
	while ((pf->precision-- != 0) && (*s != 0))
		print(pf, *s++);
	while (pf->width-- > size)
		print(pf, ' ');
}

/*******************************************************************************
 * Pour afficher les "%c".
 ******************************************************************************/
static void
print_char(struct printf_st *pf, char c) {
	if (!(pf->flags & PRINTF_LEFT_JUSTIFY))
		while (pf->width-- > 1)
			print(pf, ' ');
	print(pf, c);
	while (pf->width-- > 1)
		print(pf, ' ');
}

/*******************************************************************************
 * Pour afficher les "%x", "%X".
 ******************************************************************************/
static void
print_hexa(struct printf_st *pf, unsigned long i) {
	int pos = 0;
	int n;

	/* On ne met pas le "0x" si le nombre est nul. */
	if (i == 0)
		pf->flags &= ~PRINTF_ALTERNATE;

	/* La pf->precision par defaut pour un entier est 1. */
	if (pf->precision == -1)
		pf->precision = 1;
	else
		pf->flags &= ~PRINTF_PAD0;

	/* On ecrit l'entier dans le buffer. */
	while (i != 0) {
		n = i % 16;
		i = i / 16;

		/* On calcule le chiffre de poids faible. */
		if (n < 10)
			n += '0';
		else if (pf->flags & PRINTF_CAPITAL_X)
			n += 'A' - 10;
		else
			n += 'a' - 10;

		/* On le met en buffer. */
		pf->buffer_nombre[pos++] = n;
	}

	/* On met a jour la precision avec celle que demande le nombre affiche. */
	pf->precision = (pos > pf->precision) ? pos : pf->precision;

	/* Si on doit remplir avec des 0, on modifie la precision en consequence. */
	if ((!(pf->flags & PRINTF_LEFT_JUSTIFY)) && (pf->flags & PRINTF_PAD0)) {
		n = pf->width;

		if ((pf->flags & PRINTF_SHOW_SIGN) || (pf->flags & PRINTF_SPACE_PLUS))
			n--;
		if (pf->flags & PRINTF_ALTERNATE)
			n -= 2;
		pf->precision = (pf->precision > n) ? pf->precision : n;
		n = pf->width;
	} else {
		n = pf->precision;
		if ((pf->flags & PRINTF_SHOW_SIGN) || (pf->flags & PRINTF_SPACE_PLUS))
			n++;
		if (pf->flags & PRINTF_ALTERNATE)
			n += 2;
	}
	/* Ici n = nombre de caracteres != ' ' affiches. */

	/* Doit-on mettre des espaces de remplissage avant le nombre ? */
	if (!(pf->flags & PRINTF_LEFT_JUSTIFY)) {
		while (pf->width-- > n)
			print(pf, ' ');
	}

	/* On place eventuellement le signe. */
	if (pf->flags & PRINTF_SHOW_SIGN)
		print(pf, '+');
	else if (pf->flags & PRINTF_SPACE_PLUS)
		print(pf, ' ');

	/* On ecrit l'eventuel "0x" ou "0X". */
	if (pf->flags & PRINTF_ALTERNATE) {
		print(pf, '0');
		if (pf->flags & PRINTF_CAPITAL_X)
			print(pf, 'X');
		else
			print(pf, 'x');
	}

	/* On met les eventuels 0 de remplissage. */
	while (pf->precision-- > pos)
		print(pf, '0');

	/* On copie le reste du nombre. */
	while (pos-- != 0)
		print(pf, pf->buffer_nombre[pos]);

	/* On met enfin les eventuels espaces de fin. */
	while (pf->width-- > n)
		print(pf, ' ');
}

/*******************************************************************************
 * Pour afficher les "%d", "%i" et "%u". Le signe doit etre '+' ou '-'.
 ******************************************************************************/
static void
print_dec(struct printf_st *pf, unsigned long i, char sign) {
	int pos = 0;
	int n;

	/* La precision par defaut pour un entier est 1. */
	if (pf->precision == -1)
		pf->precision = 1;
	else
		pf->flags &= ~PRINTF_PAD0;

	/* On determine le signe a afficher. */
	if ((sign == '+') && (!(pf->flags & PRINTF_SHOW_SIGN))) {
		if (pf->flags & PRINTF_SPACE_PLUS)
			sign = ' ';
		else
			sign = 0;
	}

	/* On ecrit l'entier dans le buffer. */
	while (i != 0) {
		/* On le met en buffer. */
		pf->buffer_nombre[pos++] = (i % 10) + '0';
		i = i / 10;
	}

	/* On met a jour la precision avec celle que demande le nombre affiche. */
	pf->precision = (pos > pf->precision) ? pos : pf->precision;

	/* Si on doit remplir avec des 0, on modifie la precision en consequence. */
	if ((!(pf->flags & PRINTF_LEFT_JUSTIFY)) && (pf->flags & PRINTF_PAD0)) {
		n = pf->width;

		if (sign != 0)
			n--;
		pf->precision = (pf->precision > n) ? pf->precision : n;
		n = pf->width;
	} else {
		n = pf->precision;
		if (sign != 0)
			n++;
	}
	/* Ici n = nombre de caracteres != ' ' affiches. */

	/* Doit-on mettre des espaces de remplissage avant le nombre ? */
	if (!(pf->flags & PRINTF_LEFT_JUSTIFY)) {
		while (pf->width-- > n)
			print(pf, ' ');
	}

	/* On place eventuellement le signe. */
	if (sign != 0)
		print(pf, sign);

	/* On met les eventuels 0 de remplissage. */
	while (pf->precision-- > pos)
		print(pf, '0');

	/* On copie le reste du nombre. */
	while (pos-- != 0)
		print(pf, pf->buffer_nombre[pos]);

	/* On met enfin les eventuels espaces de fin. */
	while (pf->width-- > n)
		print(pf, ' ');
}

/*******************************************************************************
 *   Pour afficher les "%x", "%X".
 ******************************************************************************/
static void
print_oct(struct printf_st *pf, unsigned int i) {
	int pos = 0;
	int n;

	/* La precision par defaut pour un entier est 1. */
	if (pf->precision == -1)
		pf->precision = 1;
	else
		pf->flags &= ~PRINTF_PAD0;

	/* On ecrit l'entier dans le buffer. */
	while (i != 0) {
		pf->buffer_nombre[pos++] = (i % 8) + '0';
		i = i / 8;
	}

	/* On verifie si on doit mettre un zero en tete. */
	if (pf->flags & PRINTF_ALTERNATE)
		pf->buffer_nombre[pos++] = '0';

	/* On met a jour la precision avec celle que demande le nombre affiche. */
	pf->precision = (pos > pf->precision) ? pos : pf->precision;

	/* Si on doit remplir avec des 0, on modifie la precision en consequence. */
	if ((!(pf->flags & PRINTF_LEFT_JUSTIFY)) && (pf->flags & PRINTF_PAD0)) {
		n = pf->width;

		if ((pf->flags & PRINTF_SHOW_SIGN) || (pf->flags & PRINTF_SPACE_PLUS))
			n--;
		pf->precision = (pf->precision > n) ? pf->precision : n;
		n = pf->width;
	} else {
		n = pf->precision;
		if ((pf->flags & PRINTF_SHOW_SIGN) || (pf->flags & PRINTF_SPACE_PLUS))
			n++;
	}
	/* Ici n = nombre de caracteres != ' ' affiches. */

	/* Doit-on mettre des espaces de remplissage avant le nombre ? */
	if (!(pf->flags & PRINTF_LEFT_JUSTIFY)) {
		while (pf->width-- > n)
			print(pf, ' ');
	}

	/* On place eventuellement le signe. */
	if (pf->flags & PRINTF_SHOW_SIGN)
		print(pf, '+');
	else if (pf->flags & PRINTF_SPACE_PLUS)
		print(pf, ' ');

	/* On met les eventuels 0 de remplissage. */
	while (pf->precision-- > pos)
		print(pf, '0');

	/* On copie le reste du nombre. */
	while (pos-- != 0)
		print(pf, pf->buffer_nombre[pos]);

	/* On met enfin les eventuels espaces de fin. */
	while (pf->width-- > n)
		print(pf, ' ');
}

/*******************************************************************************
 * Pour afficher les "%p".
 ******************************************************************************/
static void
print_pointer(struct printf_st *pf, void *p) {
	if (p == 0) {
		print_string(pf, "(nil)");
	} else {
		pf->flags |= PRINTF_ALTERNATE;
		print_hexa(pf, (unsigned long) p);
	}
}

/*******************************************************************************
 * Voici la fonction "principale".
 ******************************************************************************/
static int
__printf(struct printf_st *pf, const char *format, va_list ap) {
	pf->count = 0;
	while (*format != 0) {
		if (*format == '%') {
			const char *ptr = format + 1;

			/* On lit le champ optionnel flags. */
			pf->flags = 0;
			flags_l:
			switch (*ptr) {
			case '-':
				pf->flags |= PRINTF_LEFT_JUSTIFY;
				ptr++;
				goto flags_l;

			case '+':
				pf->flags |= PRINTF_SHOW_SIGN;
				ptr++;
				goto flags_l;

			case ' ':
				pf->flags |= PRINTF_SPACE_PLUS;
				ptr++;
				goto flags_l;

			case '#':
				pf->flags |= PRINTF_ALTERNATE;
				ptr++;
				goto flags_l;

			case '0':
				pf->flags |= PRINTF_PAD0;
				ptr++;
				goto flags_l;
			}

			/* On lit le champ optionnel width. */
			if (*ptr == '*') {
				pf->width = va_arg(ap, int);
				ptr++;
			} else {
				pf->width = 0;
				while ((*ptr >= '0') && (*ptr <= '9'))
					pf->width =
						pf->width * 10 + (*ptr++) - '0';
			}

			/* On lit le champ optionnel de precision. */
			if (*ptr == '.') {
				ptr++;
				if (*ptr == '*') {
					pf->precision = va_arg(ap, int);
					ptr++;
				} else {
					pf->precision = 0;
					while ((*ptr >= '0')
						&& (*ptr <= '9'))
						pf->precision +=
							pf->precision * 10 +
							(*ptr++) - '0';
				}
			} else
				pf->precision = -1;

			/* On lit le champ optionnel modifier. */
			pf->modifier = 0;
			if ((*ptr == 'h') || (*ptr == 'l')
				|| (*ptr == 'L'))
				pf->modifier = *ptr++;

			/* On lit enfin le champ obligatoire. */
			switch (*ptr) {
			case 'p':
				print_pointer(pf, va_arg(ap, void *));
				break;

			case 'X':
				pf->flags |= PRINTF_CAPITAL_X;
			case 'x':
				if (pf->modifier == 'h')
					print_hexa(pf, va_arg(ap, int));
				else if (pf->modifier == 'l')
					print_hexa(pf, va_arg
							(ap, unsigned long));
				else
					print_hexa(pf, va_arg
							(ap, unsigned int));
				break;

			case 'd':
			case 'i':
				{
					int i;

					if (pf->modifier == 'h')
						i = va_arg(ap, int);
					else if (pf->modifier == 'l')
						i = va_arg(ap, long);
					else
						i = va_arg(ap, int);
					if (i < 0)
						print_dec(pf, -i, '-');
					else
						print_dec(pf, i, '+');
					break;
				}

			case 'u':
				{
					int i;

					if (pf->modifier == 'h')
						i = va_arg(ap, int);
					else if (pf->modifier == 'l')
						i = va_arg(ap, long);
					else
						i = va_arg(ap, int);
					if (i < 0)
						print_dec(pf, -i, '-');
					else
						print_dec(pf, i, '+');

					break;
				}

			case 's':
				print_string(pf, va_arg(ap, char *));
				break;

			case 'c':
				print_char(pf, va_arg(ap, int));
				break;

			case '%':
				print(pf, '%');
				break;

			case 'o':
				if (pf->modifier == 'h')
					print_oct(pf, va_arg(ap, int));
				else if (pf->modifier == 'l')
					print_oct(pf, va_arg
							(ap, unsigned long));
				else
					print_oct(pf, va_arg
							(ap, unsigned int));
				break;

			case 'n':
				*va_arg(ap, int *) = pf->count;
				break;

			case 'f':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
				pf->flags = 0;
				pf->width = 0;
				pf->precision = -1;
				print_string
					(pf, "<float format not implemented>");
				break;

			default:
				/* Comme le format n'est pas valide, on l'affiche ! */
				while (format < ptr)
					print(pf, *format++);
				ptr--;
			}
			format = ptr + 1;
		} else
			print(pf, *format++);
	}
	return pf->count;
}

static int
_printf(char *s, unsigned long n, const char *format, va_list ap)
{
	struct printf_st pf;
	pf.pos_ecr = 0;
	pf.str = s;
	pf.strl = n;

	if (s != 0) {
		/* Cas du print dans un buffer. */
		if (n) {
			/* On reserve un caractere pour le 0 terminal. */
			n--;
			__printf(&pf, format, ap);
			*s = 0;

			/* On renvoie la taille de la chaine ecrite. */
			return pf.count;
		}

		return 0;
	} else {
		/* Cas du print dans sur la console. */
		__printf(&pf, format, ap);
		pf.buffer_ecr[pf.pos_ecr] = 0;
		cons_puts(pf.buffer_ecr);

		/* On renvoie la taille de la chaine ecrite. */
		return pf.count;
	}
}

/*******************************************************************************
static int printf( const char *format, ... )
*******************************************************************************/
static int
printf(const char *format, ...)
{
	int ret;
	va_list ap;

	va_start(ap, format);
	ret = _printf(0, 0, format, ap);

	va_end(ap);
	return ret;
}

static int
assert_failed(const char *cond, const char *file, int line)
{
	printf("%s:%d: assertion '%s' failed.\n", file, line, cond);
	*(char *)0 = 0;
	exit(-1);
	while (1) ;
}

static void
cons_gets(char *s, unsigned long length)
{
	unsigned long n;
	if (length == 0) return;
	n = cons_read(s, length - 1);
	s[n] = 0;
	return;
}

/*******************************************************************************
 * Division 64 bits
 ******************************************************************************/
unsigned long long
div64(unsigned long long x, unsigned long long div, unsigned long long *rem)
{
	unsigned long long mul = 1;
	unsigned long long q;

	if ((div > x) || !div) {
		if (rem) *rem = x;
		return 0;
	}

	while (!((div >> 32) & 0x80000000ULL)) {
		unsigned long long newd = div + div;
		if (newd > x) break;
		div = newd;
		mul += mul;
	}

	q = mul;
	x -= div;
	while (1) {
		mul /= 2;
		div /= 2;
		if (!mul) {
			if (rem) *rem = x;
			return q;
		}
		if (x < div) continue;
		q += mul;
		x -= div;
	}
}

/*******************************************************************************
 * Pseudo random number generator
 ******************************************************************************/
static unsigned long long mul64(unsigned long long x, unsigned long long y)
{
	unsigned long a, b, c, d, e, f, g, h;
	unsigned long long res = 0;
	a = x & 0xffff;
	x >>= 16;
	b = x & 0xffff;
	x >>= 16;
	c = x & 0xffff;
	x >>= 16;
	d = x & 0xffff;
	e = y & 0xffff;
	y >>= 16;
	f = y & 0xffff;
	y >>= 16;
	g = y & 0xffff;
	y >>= 16;
	h = y & 0xffff;
	res = d * e;
	res += c * f;
	res += b * g;
	res += a * h;
	res <<= 16;
	res += c * e;
	res += b * f;
	res += a * g;
	res <<= 16;
	res += b * e;
	res += a * f;
	res <<= 16;
	res += a * e;
	return res;
}

typedef unsigned long long uint_fast64_t;
typedef unsigned long uint_fast32_t;

static const uint_fast64_t _multiplier = 0x5DEECE66DULL;
static const uint_fast64_t _addend = 0xB;
static const uint_fast64_t _mask = (1ULL << 48) - 1;
static uint_fast64_t _seed = 1;

// Assume that 1 <= _bits <= 32
static uint_fast32_t
randBits(int _bits)
{
	uint_fast32_t rbits;
	uint_fast64_t nextseed = (mul64(_seed, _multiplier) + _addend) & _mask;
	_seed = nextseed;
	rbits = nextseed >> 16;
	return rbits >> (32 - _bits);
}

static void
setSeed(uint_fast64_t _s)
{
	_seed = _s;
}

static unsigned long
rand()
{
	return randBits(32);
}

/*******************************************************************************
 * Unmask interrupts for those who are working in kernel mode
 ******************************************************************************/
static void test_it()
{
	__asm__ volatile("pushfl; testl $0x200,(%%esp); jnz 0f; sti; nop; cli; 0: addl $4,%%esp\n":::"memory");
}

/*******************************************************************************
 * Test 1
 *
 * Demarrage de processus avec passage de parametre
 * Terminaison normale avec valeur de retour
 * Attente de terminaison (cas fils avant pere et cas pere avant fils)
 ******************************************************************************/
static int
dummy1(void *arg)
{
	printf("1");
	assert((int) arg == DUMMY_VAL);
	return 3;
}

static int
dummy1_2(void *arg)
{
	printf(" 5");
	assert((int) arg == DUMMY_VAL + 1);

	return 4;
}

static void
test1(void)
{
	int pid1;
	int r;
	int rval;

	pid1 = start(dummy1, 4000, 192, "paramRetour", (void *) DUMMY_VAL);
	assert(pid1 > 0);
	printf(" 2");
	r = waitpid(pid1, &rval);
	assert(r == pid1);
	assert(rval == 3);
	printf(" 3");
	pid1 = start(dummy1_2, 4000, 100, "paramRetour", (void *) (DUMMY_VAL + 1));
	assert(pid1 > 0);
	printf(" 4");
	r = waitpid(pid1, &rval);
	assert(r == pid1);
	assert(rval == 4);
	printf(" 6.\n");
}

/*******************************************************************************
 * Test 2
 *
 * kill() de fils suspendu pas demarre
 * waitpid() de ce fils termine par kill()
 * waitpid() de fils termine par exit()
 ******************************************************************************/
static int
dummy2(void *args)
{
	printf(" X");
	return (int)args;
}

static int
dummy2_2(void *args)
{
	printf(" 5");
	exit((int) args);
	assert(0);
	return 0;
}

static void
test2(void)
{
	int rval;
	int r;
	int pid1;
	int val = 45;

	printf("1");
	pid1 = start(dummy2, 4000, 100, "procKill", (void *) val);
	assert(pid1 > 0);
	printf(" 2");
	r = kill(pid1);
	assert(r == 0);
	printf(" 3");
	r = waitpid(pid1, &rval);
	assert(rval == 0);
	assert(r == pid1);
	printf(" 4");
	pid1 = start(dummy2_2, 4000, 192, "procExit", (void *) val);
	assert(pid1 > 0);
	printf(" 6");
	r = waitpid(pid1, &rval);
	assert(rval == val);
	assert(r == pid1);
	assert(waitpid(getpid(), &rval) < 0);
	printf(" 7.\n");
}

/*******************************************************************************
 * Test 3
 *
 * chprio() et ordre de scheduling
 * kill() d'un processus qui devient moins prioritaire
 ******************************************************************************/
static int
proc_prio4(void *arg)
{
	/* arg = priority of this proc. */
	int r;

	assert(getprio(getpid()) == (int) arg);
	printf("1");
	r = chprio(getpid(), 64);
	assert(r == (int) arg);
	printf(" 3");
	return 0;
}

static int
proc_prio5(void *arg)
{
	/* Arg = priority of this proc. */
	int r;

	assert(getprio(getpid()) == (int) arg);
	printf(" 7");
	r = chprio(getpid(), 64);
	printf("error: I should have been killed\n");
	assert(0);
	return 0;
}

static void
test3(void)
{
	int pid1;
	int p = 192;
	int r;

	assert(getprio(getpid()) == 128);
	pid1 = start(proc_prio4, 4000, p, "prio", (void *) p);
	assert(pid1 > 0);
	printf(" 2");
	r = chprio(getpid(), 32);
	assert(r == 128);
	printf(" 4");
	r = chprio(getpid(), 128);
	assert(r == 32);
	printf(" 5");
	assert(waitpid(pid1, 0) == pid1);
	printf(" 6");

	assert(getprio(getpid()) == 128);
	pid1 = start(proc_prio5, 4000, p, "prio", (void *) p);
	assert(pid1 > 0);
	printf(" 8");
	r = kill(pid1);
	assert(r == 0);
	assert(waitpid(pid1, 0) == pid1);
	printf(" 9");
	r = chprio(getpid(), 32);
	assert(r == 128);
	printf(" 10");
	r = chprio(getpid(), 128);
	assert(r == 32);
	printf(" 11.\n");
}

/*******************************************************************************
 * Test 4
 *
 * Boucles d'attente active (partage de temps)
 * chprio()
 * kill() de processus de faible prio
 * kill() de processus deja mort
 ******************************************************************************/
static const int loop_count0 = 5000;
static const int loop_count1 = 10000;

static int
busy_loop1(void *arg)
{
	while (1) {
		int i, j;

		printf(" A");
		for (i=0; i<loop_count1; i++) {
			test_it();
			for (j=0; j<loop_count0; j++);
		}
	}
	return 0;
}

/* assume the process to suspend has a priority == 64 */
static int
busy_loop2(void *arg)
{
	int i;

	for (i = 0; i < 3; i++) {
		int k, j;

		printf(" B");
		for (k=0; k<loop_count1; k++) {
			test_it();
			for (j=0; j<loop_count0; j++);
		}
	}
	i = chprio((int) arg, 16);
	assert(i == 64);
	return 0;
}

static void
test4(void)
{
	int pid1, pid2;
	int r;
	int arg = 0;

	assert(getprio(getpid()) == 128);
	pid1 = start(busy_loop1, 4000, 64, "busy1", (void *) arg);
	assert(pid1 > 0);
	pid2 = start(busy_loop2, 4000, 64, "busy2", (void *) pid1);
	assert(pid2 > 0);
	printf("1 -");
	r = chprio(getpid(), 32);
	assert(r == 128);
	printf(" - 2");
	r = kill(pid1);
	assert(r == 0);
	assert(waitpid(pid1, 0) == pid1);
	r = kill(pid2);
	assert(r < 0); /* kill d'un processus zombie */
	assert(waitpid(pid2, 0) == pid2);
	printf(" 3");
	r = chprio(getpid(), 128);
	assert(r == 32);
	printf(" 4.\n");
}

/*******************************************************************************
 * Test 5
 *
 * Tests de quelques parametres invalides.
 * Certaines interdictions ne sont peut-etre pas dans la spec. Changez les pour
 * faire passer le test correctement.
 ******************************************************************************/
static int
no_run(void *arg)
{
	assert(0);
	return 1;
}

static int
waiter(void *arg)
{
	int pid = (int)arg;
	assert(kill(pid) == 0);
	assert(waitpid(pid, 0) < 0);
	return 1;
}

static void
test5(void)
{
	int pid1, pid2;
	int r;

	// Le processus 0 et la priorite 0 sont des parametres invalides
	assert(kill(0) < 0);
	assert(chprio(getpid(), 0) < 0);
	assert(getprio(getpid()) == 128);
	pid1 = start(no_run, 4000, 64, "norun", 0);
	assert(pid1 > 0);
	assert(kill(pid1) == 0);
	assert(kill(pid1) < 0); //pas de kill de zombie
	assert(chprio(pid1, 128) < 0); //changer la priorite d'un zombie
	assert(chprio(pid1, 64) < 0); //changer la priorite d'un zombie
	assert(waitpid(pid1, 0) == pid1);
	assert(waitpid(pid1, 0) < 0);
	pid1 = start(no_run, 4000, 64, "norun", 0);
	assert(pid1 > 0);
	pid2 = start(waiter, 4000, 65, "waiter", (void *)pid1);
	assert(pid2 > 0);
	assert(waitpid(pid2, &r) == pid2);
	assert(r == 1);
	assert(waitpid(pid1, &r) == pid1);
	assert(r == 0);
	printf("ok.\n");
}

/*******************************************************************************
 * Test 6
 *
 * Waitpid multiple.
 * Creation de processus avec differentes tailles de piles.
 ******************************************************************************/
extern int __proc6_1(void *arg);
extern int __proc6_2(void *arg);

__asm__(
".text\n"
".globl __proc6_1\n"
"__proc6_1:\n"
"movl $3,%eax\n"
"ret\n"
".globl __proc6_2\n"
"__proc6_2:\n"
"movl 4(%esp),%eax\n"
"pushl %eax\n"
"popl %eax\n"
"ret\n"
".previous\n"
);

static void
test6(void)
{
	int pid1, pid2, pid3;
	int ret;

	assert(getprio(getpid()) == 128);
	pid1 = start(__proc6_1, 0, 64, "proc6_1", 0);
	assert(pid1 > 0);
	pid2 = start(__proc6_2, 4, 66, "proc6_2", (void*)4);
	assert(pid2 > 0);
	pid3 = start(__proc6_2, 0xffffffff, 65, "proc6_3", (void*)5);
	assert(pid3 < 0);
	pid3 = start(__proc6_2, 8, 65, "proc6_3", (void*)5);
	assert(pid3 > 0);
	assert(waitpid(-1, &ret) == pid2);
	assert(ret == 4);
	assert(waitpid(-1, &ret) == pid3);
	assert(ret == 5);
	assert(waitpid(-1, &ret) == pid1);
	assert(ret == 3);
	assert(waitpid(pid1, 0) < 0);
	assert(waitpid(-1, 0) < 0);
	assert(waitpid(getpid(), 0) < 0);
	printf("ok.\n");
}

/*******************************************************************************
 * Test 7
 *
 * Test de l'horloge (ARR et ACE)
 * Tentative de determination de la frequence du processeur et de la
 * periode de scheduling
 ******************************************************************************/

static int
proc_timer1(void *arg)
{
	unsigned long quartz;
	unsigned long ticks;
	unsigned long dur;
	int i;

	clock_settings(&quartz, &ticks);
	dur = (quartz + ticks) / ticks;
	printf(" 2");
	for (i = 4; i < 8; i++) {
		wait_clock(current_clock() + dur);
		printf(" %d", i);
	}
	return 0;
}

static volatile unsigned long timer;

static int
proc_timer(void *arg)
{
	while (1) {
		unsigned long t = timer + 1;
		timer = t;
		while (timer == t) test_it();
	}
	while (1);
	return 0;
}

static int
sleep_pr1(void *args)
{
	wait_clock(current_clock() + 2);
	printf(" not killed !!!");
	assert(0);
	return 1;
}

static void
test7(void)
{
	int pid1, pid2, r;
	unsigned long c0, c, quartz, ticks, dur;

	assert(getprio(getpid()) == 128);
	printf("1");
	pid1 = start(proc_timer1, 4000, 129, "timer", 0);
	assert(pid1 > 0);
	printf(" 3");
	assert(waitpid(-1, 0) == pid1);
	printf(" 8 : ");

	timer = 0;
	pid1 = start(proc_timer, 4000, 127, "timer1", 0);
	pid2 = start(proc_timer, 4000, 127, "timer2", 0);
	assert(pid1 > 0);
	assert(pid2 > 0);
	clock_settings(&quartz, &ticks);
	dur = 2 * quartz / ticks;
	test_it();
	c0 = current_clock();
	do {
		test_it();
		c = current_clock();
	} while (c == c0);
	wait_clock(c + dur);
	assert(kill(pid1) == 0);
	assert(waitpid(pid1, 0) == pid1);
	assert(kill(pid2) == 0);
	assert(waitpid(pid2, 0) == pid2);
	printf("%lu changements de contexte sur %lu tops d'horloge", timer, dur);
	pid1 = start(sleep_pr1, 4000, 192, "sleep", 0);
	assert(pid1 > 0);
	assert(kill(pid1) == 0);
	assert(waitpid(pid1, &r) == pid1);
	assert(r == 0);
	printf(".\n");
}

/*******************************************************************************
 * Test 8
 *
 * Creation de processus se suicidant en boucle. Test de la vitesse de creation
 * de processus.
 ******************************************************************************/
static int
suicide(void *arg)
{
	kill(getpid());
	assert(0);
	return 0;
}

static int
suicide_launcher(void *arg)
{
	int pid1;
	pid1 = start(suicide, 4000, 192, "suicide_launcher", 0);
	assert(pid1 > 0);
	return pid1;
}

static void
test8(void)
{
	unsigned long long tsc1;
	unsigned long long tsc2;
	int i, r, pid, count;

	assert(getprio(getpid()) == 128);

	/* Le petit-fils va passer zombie avant le fils mais ne pas
	etre attendu par waitpid. Un nettoyage automatique doit etre
	fait. */
	pid = start(suicide_launcher, 4000, 129, "suicide_launcher", 0);
	assert(pid > 0);
	assert(waitpid(pid, &r) == pid);
	assert(chprio(r, 192) < 0);

	count = 0;
	__asm__ __volatile__("rdtsc":"=A"(tsc1));
	do {
		for (i=0; i<10; i++) {
			pid = start(suicide_launcher, 4000, 200, "suicide_launcher", 0);
			assert(pid > 0);
			assert(waitpid(pid, 0) == pid);
		}
		test_it();
		count += i;
		__asm__ __volatile__("rdtsc":"=A"(tsc2));
	} while ((tsc2 - tsc1) < 1000000000);
	printf("%lu cycles/process.\n", (unsigned long)div64(tsc2 - tsc1, 2 * count, 0));
}

/*******************************************************************************
 * Test 9
 *
 * Test de la sauvegarde des registres dans les appels systeme et interruptions
 ******************************************************************************/
static int
nothing(void *arg)
{
	return 0;
}

int __err_id = 0;

extern void
__test_error(void)
{
	(void)nothing;
	printf("assembly check failed, id = %d\n", __err_id);
	exit(1);
}

__asm__(
".text\n"
".globl __test_valid_regs1\n"
"__test_valid_regs1:\n"

"pushl %ebp; movl %esp, %ebp; pushal\n"
"movl 8(%ebp),%ebx\n"
"movl 12(%ebp),%edi\n"
"movl 16(%ebp),%esi\n"
"movl %ebp,1f\n"
"movl %esp,2f\n"

"pushl $0\n"
"pushl $3f\n"
"pushl $192\n"
"pushl $4000\n"
"pushl $nothing\n"
"call start\n"
"addl $20,%esp\n"
"movl $1,__err_id\n"
"testl %eax,%eax\n"
"jle 0f\n"
"pushl %eax\n"
"pushl $0\n"
"pushl %eax\n"
"call waitpid\n"
"addl $8,%esp\n"
"popl %ecx\n"
"movl $3,__err_id\n"
"cmpl %ecx,%eax\n"
"jne 0f\n"

"movl $4,__err_id\n"
"cmpl %esp,2f\n"
"jne 0f\n"
"movl $5,__err_id\n"
"cmpl %ebp,1f\n"
"jne 0f\n"
"movl $6,__err_id\n"
"cmpl 8(%ebp),%ebx\n"
"jne 0f\n"
"movl $7,__err_id\n"
"cmpl 12(%ebp),%edi\n"
"jne 0f\n"
"movl $8,__err_id\n"
"cmpl 16(%ebp),%esi\n"
"jne 0f\n"
"popal; leave\n"
"ret\n"
"0: jmp __test_error\n"
"0:\n"
"jmp 0b\n"
".previous\n"
".data\n"
"1: .long 0x12345678\n"
"2: .long 0x87654321\n"
"3: .string \"nothing\"\n"
".previous\n"
);

extern void
__test_valid_regs1(int a1, int a2, int a3);

__asm__(
".text\n"
".globl __test_valid_regs2\n"
"__test_valid_regs2:\n"

"pushl %ebp; movl %esp, %ebp; pushal\n"

"movl 8(%ebp),%eax\n"
"movl 12(%ebp),%ebx\n"
"movl 16(%ebp),%ecx\n"
"movl 20(%ebp),%edx\n"
"movl 24(%ebp),%edi\n"
"movl 28(%ebp),%esi\n"
"movl %ebp,1f\n"
"movl %esp,2f\n"

"movl $1,__err_id\n"
"3: testl $1,__it_ok\n"
"jnz 0f\n"

"3: pushfl\n"
"testl $0x200,(%esp)\n"
"jnz 4f\n"
"sti\n"
"nop\n"
"cli\n"
"4:\n"
"addl $4,%esp\n"
"testl $1,__it_ok\n"
"jz 3b\n"

"movl $2,__err_id\n"
"cmpl %esp,2f\n"
"jne 0f\n"
"movl $3,__err_id\n"
"cmpl %ebp,1f\n"
"jne 0f\n"
"movl $4,__err_id\n"
"cmpl 8(%ebp),%eax\n"
"jne 0f\n"
"movl $5,__err_id\n"
"cmpl 12(%ebp),%ebx\n"
"jne 0f\n"
"movl $6,__err_id\n"
"cmpl 16(%ebp),%ecx\n"
"jne 0f\n"
"movl $7,__err_id\n"
"cmpl 20(%ebp),%edx\n"
"jne 0f\n"
"movl $8,__err_id\n"
"cmpl 24(%ebp),%edi\n"
"jne 0f\n"
"movl $9,__err_id\n"
"cmpl 28(%ebp),%esi\n"
"jne 0f\n"
"popal; leave\n"
"ret\n"
"0: jmp __test_error\n"
"0:\n"
"jmp 0b\n"
".previous\n"
".data\n"
"1: .long 0x12345678\n"
"2: .long 0x87654321\n"
".previous\n"
);

static volatile int __it_ok;

extern void
__test_valid_regs2(int a1, int a2, int a3, int a4, int a5, int a6);

static int
test_regs2(void *arg)
{
	__it_ok = 0;
	__test_valid_regs2(rand(), rand(), rand(), rand(), rand(), rand());
	return 0;
}

static void
test9(void)
{
	int i;
	assert(getprio(getpid()) == 128);
	printf("1");
	for (i=0; i<1000; i++) {
		__test_valid_regs1(rand(), rand(), rand());
	}
	printf(" 2");
	for (i=0; i<25; i++) {
		int pid;
		__it_ok = 1;
		pid = start(test_regs2, 4000, 128, "test_regs2", 0);
		assert(pid > 0);
		while (__it_ok) test_it();
		__it_ok = 1;
		assert(waitpid(pid, 0) == pid);
	}
	printf(" 3.\n");
}

/*******************************************************************************
 * Test 10
 *
 * Test d'utilisation d'un semaphore comme simple compteur.
 ******************************************************************************/
static void
test10(void)
{
	int sem1 = screate(2);
	assert(sem1 >= 0);
	assert(scount(sem1) == 2);
	assert(signal(sem1) == 0);
	assert(scount(sem1) == 3);
	assert(signaln(sem1, 2) == 0);
	assert(scount(sem1) == 5);
	assert(wait(sem1) == 0);
	assert(scount(sem1) == 4);
	assert(sreset(sem1, 7) == 0);
	assert(scount(sem1) == 7);
	assert(sdelete(sem1) == 0);
	printf("ok.\n");
}

/*******************************************************************************
 * Test 11
 *
 * Mutex avec un semaphore, regle de priorite sur le mutex.
 ******************************************************************************/
struct sem {
	int sem;
};

static void
xwait(struct sem *s)
{
	assert(wait(s->sem) == 0);
}

static void
xsignal(struct sem *s)
{
  assert(signal(s->sem) == 0);
}

static void
xscreate(struct sem *s)
{
	assert((s->sem = screate(0)) >= 0);
}

static void
xsdelete(struct sem *s)
{
	assert(sdelete(s->sem) == 0);
}

static int in_mutex = 0;

static int
proc_mutex(void *arg)
{
	struct sem *sem = arg;
	int p = getprio(getpid());
	int msg;

	switch (p) {
	case 130:
		msg = 2;
		break;
	case 132:
		msg = 3;
		break;
	case 131:
		msg = 4;
		break;
	case 129:
		msg = 5;
		break;
	default:
		msg = 15;
	}
	printf(" %d", msg);
	xwait(sem);
	printf(" %d", 139 - p);
	assert(!(in_mutex++));
	chprio(getpid(), 16);
	chprio(getpid(), p);
	in_mutex--;
	xsignal(sem);
	return 0;
}

static void
test11(void)
{
	struct sem sem;
	int pid1, pid2, pid3, pid4;

	assert(getprio(getpid()) == 128);
	xscreate(&sem);
	printf("1");
	pid1 = start(proc_mutex, 4000, 130, "proc_mutex", &sem);
	pid2 = start(proc_mutex, 4000, 132, "proc_mutex", &sem);
	pid3 = start(proc_mutex, 4000, 131, "proc_mutex", &sem);
	pid4 = start(proc_mutex, 4000, 129, "proc_mutex", &sem);
	assert(pid1 > 0);
	assert(pid2 > 0);
	assert(pid3 > 0);
	assert(pid4 > 0);
	assert(chprio(getpid(), 160) == 128);
	printf(" 6");
	xsignal(&sem);
	assert(waitpid(-1, 0) == pid2);
	assert(waitpid(-1, 0) == pid3);
	assert(waitpid(-1, 0) == pid1);
	assert(waitpid(-1, 0) == pid4);
	assert(waitpid(-1, 0) < 0);
	assert(chprio(getpid(), 128) == 160);
	xsdelete(&sem);
	printf(" 11.\n");
}

/*******************************************************************************
 * Test 12
 *
 * Deblocages par signaln, atomicite.
 ******************************************************************************/
static int
proc12_1(void *arg)
{
	int sem = (int)arg;
	assert(try_wait(sem) == 0);
	assert(try_wait(sem) == -3);
	printf("1");
	assert(wait(sem) == 0);
	printf(" 8");
	assert(wait(sem) == 0);
	printf(" 11");
	exit(1);
}

static int
proc12_2(void *arg)
{
	int sem = (int)arg;
	printf(" 5");
	assert(wait(sem) == 0);
	printf(" 13");
	return 2;
}

static int
proc12_3(void *arg)
{
	int sem = (int)arg;
	printf(" 3");
	assert(wait(sem) == 0);
	printf(" 7");
	assert(wait(sem) == 0);
	printf(" 9");
	assert(wait(sem) == 0);
	printf(" 10");
	kill(getpid());
	assert(!"Should not arrive here !");
	while(1);
	return 0;
}

static void
test12(void)
{
	int sem;
	int pid1, pid2, pid3;
	int ret;

	assert(getprio(getpid()) == 128);
	assert((sem = screate(1)) >= 0);
	pid1 = start(proc12_1, 4000, 129, "proc12_1", (void *)sem);
	assert(pid1 > 0);
	printf(" 2");
	pid2 = start(proc12_2, 4000, 127, "proc12_2", (void *)sem);
	assert(pid2 > 0);
	pid3 = start(proc12_3, 4000, 130, "proc12_3", (void *)sem);
	assert(pid3 > 0);
	printf(" 4");
	assert(chprio(getpid(), 126) == 128);
	printf(" 6");
	assert(chprio(getpid(), 128) == 126);
	assert(signaln(sem, 2) == 0);
	assert(signaln(sem, 1) == 0);
	assert(signaln(sem, 4) == 0);
	assert(waitpid(pid1, &ret) == pid1);
	assert(ret == 1);
	assert(waitpid(-1, &ret) == pid3);
	assert(ret == 0);
	assert(scount(sem) == 1);
	assert(sdelete(sem) == 0);
	printf(" 12");
	assert(waitpid(-1, &ret) == pid2);
	assert(ret == 2);
	printf(" 14.\n");
}

/*******************************************************************************
 * Test 13
 *
 * sreset, sdelete
 ******************************************************************************/
static int
proc13_1(void *arg)
{
	int sem = (int)arg;
	assert(try_wait(sem) == 0);
	assert(try_wait(sem) == -3);
	printf("1");
	assert(wait(sem) == -4);
	printf(" 9");
	assert(wait(sem) == -3);
	printf(" 13");
	assert(wait(sem) == -1);

	exit(1);
}

static int
proc13_2(void *arg)
{
  int sem = (int)arg;
  printf(" 5");
  assert(wait(sem) == -4);
  printf(" 11");
  assert(wait(sem) == -3);
  printf(" 15");
  assert(wait(sem) == -1);
  return 2;
}

static int
proc13_3(void *arg)
{
	int sem = (int)arg;
	printf(" 3");
	assert(wait(sem) == -4);
	printf(" 7");
	assert(wait(sem) == 0);
	printf(" 8");
	assert(wait(sem) == -3);
	printf(" 12");
	assert(wait(sem) == -1);
	exit(3);
	assert(!"Should not arrive here !");
	while(1);
}

static void
test13(void)
{
	int sem;
	int pid1, pid2, pid3;
	int ret;

	assert(getprio(getpid()) == 128);
	assert((sem = screate(1)) >= 0);
	pid1 = start(proc13_1, 4000, 129, "proc13_1", (void *)sem);
	assert(pid1 > 0);
	printf(" 2");
	pid2 = start(proc13_2, 4000, 127, "proc13_2", (void *)sem);
	assert(pid2 > 0);
	pid3 = start(proc13_3, 4000, 130, "proc13_3", (void *)sem);
	assert(pid3 > 0);
	printf(" 4");
	assert(chprio(getpid(), 126) == 128);
	printf(" 6");
	assert(chprio(getpid(), 128) == 126);
	assert(sreset(sem, 1) == 0);
	printf(" 10");
	assert(chprio(getpid(), 126) == 128);
	assert(chprio(getpid(), 128) == 126);
	assert(sdelete(sem) == 0);
	printf(" 14");
	assert(waitpid(pid1, &ret) == pid1);
	assert(ret == 1);
	assert(waitpid(-1, &ret) == pid3);
	assert(ret == 3);
	assert(waitpid(-1, &ret) == pid2);
	assert(ret == 2);
	assert(signal(sem) == -1);
	assert(scount(sem) == -1);
	assert(sdelete(sem) == -1);
	printf(" 16.\n");
}

/*******************************************************************************
 * Test 14
 *
 * chprio et kill de processus bloque sur semaphore
 ******************************************************************************/
static int
proc14_1(void *arg)
{
	int sem = (int)arg;
	printf("1");
	assert(wait(sem) == 0);
	printf(" 5");
	assert(wait(sem) == 0);
	printf(" 11");
	exit(1);
}

static int proc14_2(void *arg)
{
  int sem = (int)arg;
  printf(" 3");
  assert(wait(sem) == 0);
  printf(" 7");
  assert(wait(sem) == 0);
  printf(" 9");
  assert(wait(sem) == 0);
  printf(" X");
  return 2;
}

static void
test14(void)
{
	int sem;
	int pid1, pid2;

	assert(getprio(getpid()) == 128);
	assert((sem = screate(0)) >= 0);
	pid1 = start(proc14_1, 4000, 129, "proc14_1", (void *)sem);
	assert(pid1 > 0);
	printf(" 2");
	pid2 = start(proc14_2, 4000, 130, "proc14_2", (void *)sem);
	assert(pid2 > 0);
	printf(" 4");
	assert(chprio(pid1, 131) == 129);
	assert(signal(sem) == 0);
	printf(" 6");
	assert(chprio(pid1, 127) == 131);
	assert(signal(sem) == 0);
	printf(" 8");
	assert(signaln(sem, 2) == 0);
	printf(" 10");
	assert(waitpid(pid1, 0) == pid1);
	assert(scount(sem) == 0xffff);
	assert(kill(pid2) == 0);
	assert(getprio(pid2) < 0);
	assert(chprio(pid2, 129) < 0);
	assert(scount(sem) == 0);
	assert(signal(sem) == 0);
	assert(scount(sem) == 1);
	assert(sdelete(sem) == 0);
	assert(waitpid(-1, 0) == pid2);
	printf(" 12.\n");
}

/*******************************************************************************
 * Test 15
 *
 * Controles d'erreurs
 ******************************************************************************/
static void
test15(void)
{
	int sem, i;

	assert(screate(-2) == -1);
	assert((sem = screate(2)) >= 0);
	assert(signaln(sem, -4) < 0);
	assert(sreset(sem, -3) == -1);
	assert(scount(sem) == 2);
	assert(signaln(sem, 32760) == 0);
	assert(signaln(sem, 6) == -2);
	assert(scount(sem) == 32762);
	assert(wait(sem) == 0);
	assert(scount(sem) == 32761);
	assert(signaln(sem, 30000) == -2);
	assert(scount(sem) == 32761);
	assert(wait(sem) == 0);
	assert(scount(sem) == 32760);
	assert(signaln(sem, -2) < 0);
	assert(scount(sem) == 32760);
	assert(wait(sem) == 0);
	assert(scount(sem) == 32759);
	assert(signaln(sem, 8) == 0);
	assert(scount(sem) == 32767);
	assert(signaln(sem, 1) == -2);
	assert(scount(sem) == 32767);
	assert(signal(sem) == -2);
	assert(scount(sem) == 32767);
	for (i=0; i<32767; i++) {
		assert(wait(sem) == 0);
	}
	assert(try_wait(sem) == -3);
	assert(scount(sem) == 0);
	assert(sdelete(sem) == 0);
	printf("ok.\n");
}

/*******************************************************************************
 * Test 16
 *
 * Allocation performance.
 ******************************************************************************/
static unsigned long
test16_1(void)
{
	unsigned long long tsc, tsc1, tsc2;
	unsigned long count = 0;

	__asm__ __volatile__("rdtsc":"=A"(tsc1));
	tsc2 = tsc1 + 1000000000;
	assert(tsc1 < tsc2);
	do {
		int i;
		test_it();
		for (i=0; i<100; i++) {
			int sem1 = screate(2);
			int sem2 = screate(1);
			assert(sem1 >= 0);
			assert(sem2 >= 0);
			assert(sem1 != sem2);
			assert(sdelete(sem1) == 0);
			assert(sdelete(sem2) == 0);
		}
		__asm__ __volatile__("rdtsc":"=A"(tsc));
		count += 2 * i;
		
	} while (tsc < tsc2);
	return (unsigned long)div64(tsc - tsc1, count, 0);
}

static short randShort()
{
	return randBits(15);
}

static int
proc16_1(void *arg)
{
	int sems[NBSEMS];
	int i;
	unsigned long c1, c2;
	unsigned long long seed;

	c1 = test16_1();
	printf("%lu ", c1);
	__asm__ __volatile__("rdtsc":"=A"(seed));
	setSeed(seed);
	for (i=0; i<NBSEMS; i++) {
		int sem = screate(randShort());
		if (sem < 0) assert(!"*** Increase the semaphore capacity of your system to NBSEMS to pass this test. ***");
		sems[i] = sem;
	}
	if (screate(0) >= 0) assert(!"*** Decrease the semaphore capacity of your system to NBSEMS to pass this test. ***");
	assert(sdelete(sems[NBSEMS/3]) == 0);
	assert(sdelete(sems[(NBSEMS/3)*2]) == 0);
	c2 = test16_1();
	printf("%lu ", c2);
	setSeed(seed);
	for (i=0; i<NBSEMS; i++) {
		short randVal = randShort();
		if ((i != (NBSEMS/3)) && (i != (2*(NBSEMS/3)))) {
			assert(scount(sems[i]) == randVal);
			assert(sdelete(sems[i]) == 0);
		}
	}
	if (c2 < 2 * c1)
		printf("ok.\n");
	else
		printf("Bad algorithm complexity in semaphore allocation.\n");
	return 0;
}

static void
test16(void)
{
	int pid = start(proc16_1, 4000 + NBSEMS * 4, 128, "proc16_1", 0);
	assert(pid > 0);
	assert(waitpid(pid, 0) == pid);
}

/*******************************************************************************
 * Test 17
 *
 * Un exemple de producteur/consommateur
 * On peut aussi faire sans operation atomique
 ******************************************************************************/
struct test17_buf_st {
	int mutex;
	int wsem;
	int wpos;
	int rsem;
	int rpos;
	char buf[100];
	int received[256];
};

static void
buf_send(char x, struct test17_buf_st *st)
{
	assert(wait(st->wsem) == 0);
	assert(wait(st->mutex) == 0);
	st->buf[(st->wpos++) % sizeof(st->buf)] = x;
	assert(signal(st->mutex) == 0);
	assert(signal(st->rsem) == 0);
}

static int
buf_receive(struct test17_buf_st *st)
{
  int x;
  assert(wait(st->rsem) == 0);
  assert(wait(st->mutex) == 0);
  x = 0xff & (int)(st->buf[(st->rpos++) % sizeof(st->buf)]);
  assert(signal(st->mutex) == 0);
  assert(signal(st->wsem) == 0);
  return x;
}

static int
proc17_1(void *arg)
{
	struct test17_buf_st *st = arg;
	unsigned long long tsc, tsc2;
	int count;

	__asm__ __volatile__("rdtsc":"=A"(tsc));
	tsc2 = tsc + 1000000000;
	assert(tsc < tsc2);
	do {
		int j;
		for (j=0; j<256; j++) {
			buf_send(j, st);
		}
		count++;
		__asm__ __volatile__("rdtsc":"=A"(tsc));
	} while (tsc < tsc2);
	return count;
}

// Increment a variable in a single atomic operation
static __inline__ void
atomic_incr(int *atomic)
{
	__asm__ __volatile__("incl %0" : "+m" (*atomic) : : "cc");
}

static int
proc17_2(void *arg)
{
	struct test17_buf_st *st = arg;

	while(1) {
		int x = buf_receive(st);
		atomic_incr(&st->received[x]);
	}
	return 0;
}

static void
test17(void)
{
	int pid[6];
	int i;
	struct test17_buf_st st;
	int count = 0;

	assert(getprio(getpid()) == 128);
	st.mutex = screate(1);
	assert(st.mutex >= 0);
	st.wsem = screate(100);
	assert(st.wsem >= 0);
	st.wpos = 0;
	st.rsem = screate(0);
	assert(st.rsem >= 0);
	st.rpos = 0;
	for (i=0; i<256; i++) {
		st.received[i] = 0;
	}
	for (i=0; i<3; i++) {
		pid[i] = start(proc17_1, 4000, 129, "proc17_1", &st);
		assert(pid[i] > 0);
	}
	for (i=3; i<6; i++) {
		pid[i] = start(proc17_2, 4000, 129, "proc17_2", &st);
		assert(pid[i] > 0);
	}
	for (i=0; i<3; i++) {
		int ret;
		assert(waitpid(pid[i], &ret) == pid[i]);
		count += ret;
	}
	printf("[2111] : scount(st.rsem) =%d\n",scount(st.rsem));
	assert(scount(st.rsem) == 0xfffd);
	for (i=3; i<6; i++) {
		int ret;
		assert(kill(pid[i]) == 0);
		assert(waitpid(pid[i], &ret) == pid[i]);
	}
	assert(scount(st.mutex) == 1);
	assert(scount(st.wsem) == 100);
	printf("[2120] : scount(st.rsem) =%d\n",scount(st.rsem));
	assert(scount(st.rsem) == 0);
	assert(sdelete(st.mutex) == 0);
	assert(sdelete(st.wsem) == 0);
	assert(sdelete(st.rsem) == 0);
	for (i=0; i<256; i++) {
		int n = st.received[i];
		if (n != count) {
			printf("st.received[%d] == %d, count == %d\n", i, n, count);
			assert(n == count);
		}
	}
	printf("ok (%d chars sent).\n", count * 256);
}

/*******************************************************************************
 * Test 18
 *
 * Amusement : piratage !
 ******************************************************************************/
static char callhack[] = { 0xcd, 0x32, 0xc3 };

__asm__(
".text\n"
".globl __hacking\n"
"__hacking:\n"

"pushal\n"
"pushl %ds\n"
"pushl %es\n"
"movl $0x18,%eax\n"
"movl %eax,%ds\n"
"movl %eax,%es\n"
"cld\n"
"call __hacking_c\n"
"popl %es\n"
"popl %ds\n"
"popal\n"
"iret\n"
);

extern void
__hacking(void);

static void __inline__
outb(unsigned char value, unsigned short port)
{
	__asm__ __volatile__("outb %0, %1" : : "a" (value), "Nd" (port));
}

static unsigned char __inline__
inb(unsigned short port)
{
	unsigned char rega;
	__asm__ __volatile__("inb %1,%0" : "=a" (rega) : "Nd" (port));
	return rega;
}

static int
getpos(void)
{
	int pos;
	outb(0x0f, 0x3d4);
	pos = inb(0x3d4 + 1);
	outb(0x0e, 0x3d4);
	pos += inb(0x3d4 + 1) << 8;
	return pos;
}

static int firsttime = 1;

void
__hacking_c(void)
{
	static int pos;
	if (firsttime) {
		firsttime = 0;
		pos = getpos();
	} else {
		int pos2 = getpos();
		char *str = "          Kernel hacked ! :P          ";
		short *ptr = (short *)0xb8000;
		int p = pos;
		while (p > pos2) p-= 80;
		if ((p < 0) || (p >= 80 * 24)) p = 80 * 23;
		ptr += p;
		while (*str) {
			*ptr++ = ((128 + 4 * 16 + 15) << 8) + *str++;
		}
	}
}

static void
do_hack(void)
{
	firsttime = 1;
	((void (*)(void))callhack)();
	printf("nok.\n");
	((void (*)(void))callhack)();
}

static int
proc_18_1(void *args)
{
	printf("1 ");
	return (int)args;
}

static int
proc_18_2(void *args)
{
	printf("2 ");
	return (int)args;
}

static void
test18(void)
{
	unsigned long a = (unsigned long)__hacking;
	unsigned long a1 = 0x100000 + (a & 0xffff);
	unsigned long a2 = (a & 0xffff0000) + 0xee00;
	int pid1, pid2;
	int cs;

	__asm__ volatile ("movl %%cs,%%eax":"=a" (cs));
	if ((cs & 3) == 0) {
		printf("This test can not work at kernel level.\n");
		return;
	}
	pid1 = start(proc_18_1, 4000, 127, "proc_18_1", (void *)a1);
	pid2 = start(proc_18_2, 4000, 126, "proc_18_2", (void *)a2);
	assert(pid1 > 0);
	assert(pid2 > 0);
	if ((waitpid(pid1, (int *)0x1190) == pid1) &&
		(waitpid(pid2, (int *)0x1194) == pid2)) {
		do_hack();
		return;
	}
	waitpid(-1, 0);
	waitpid(-1, 0);
	cons_write((char *)0x100000, 50);
	assert(start(dummy2, 4000, 100, (void *)0x100000, 0) < 0);
	printf("3.\n");
}

/*******************************************************************************
 * Test 19
 *
 * Test du clavier.
 ******************************************************************************/
static int
cons_rd0(void *arg)
{
	unsigned long i;
	char buf[101];

	i = cons_read(buf, 100);
	buf[i] = 0;
	printf(". : %s\n", buf);
	return 0;
}

static int
cons_rdN(void *arg)
{
	unsigned long i;
	char buf[101];

	i = cons_read(buf, 100);
	buf[i] = 0;
	printf("%d : %s\n", 133 - getprio(getpid()), buf);
	return 0;
}

static void
mega_cycles(int n)
{
	unsigned long long t1, t2;
	int i = 0;

	__asm__ __volatile__("rdtsc":"=A"(t1));
	for (i=0; i<n; i++) {
		do {
			test_it();
			__asm__ __volatile__("rdtsc":"=A"(t2));
		} while ((t2 - t1) < 1000000);
		t1 += 1000000;
	}
}

static void
test19(void)
{
	char ch[101];
	int i;
	unsigned long j;
	int pid1, pid2, pid3, pid4;

	printf("cons_read bloquant, entrez des caracteres : ");
	j = cons_read(ch, 100); ch[j] = 0;
	printf("%lu chars : %s\n", j, ch);
	printf("Frappez une ligne de 5 caracteres : ");
	j = cons_read(ch,5); ch[5] = 0;
	printf("%lu chars : %s\n", j, ch);
	j = cons_read(ch,5);

	if (j != 0) printf("!!! j aurait du etre nul\n");
	assert(cons_read(ch, 0) == 0);

	printf("Les tests suivants supposent un tampon clavier de l'ordre  de 20 caracteres.\nEntrez des caracteres tant qu'il y a echo, frappez quelques touches\nsupplementaires puis fin de ligne : ");
	j = cons_read(ch, 100); ch[j] = 0;
	printf("%lu chars : %s\n", j, ch);

	printf("Entrees sorties par anticipation. Frappez 4 lignes en veillant a ne pas\ndepasser la taille du tampon clavier : ");
	for (i=0; i<80; i++) {
		mega_cycles(200);
		printf(".");
	}
	printf("Fini.\n");
	pid1 = start(cons_rd0, 4000, 129, "cons_rd0", 0);
	pid2 = start(cons_rd0, 4000, 129, "cons_rd0", 0);
	pid3 = start(cons_rd0, 4000, 129, "cons_rd0", 0);
	pid4 = start(cons_rd0, 4000, 129, "cons_rd0", 0);
	waitpid(pid2, 0);
	waitpid(pid3, 0);
	waitpid(pid1, 0);
	waitpid(pid4, 0);

	printf("Entrees sorties par anticipation. Depassez maintenant la taille du tampon\nclavier : ");
	for (i=0; i<80; i++) {
		mega_cycles(200);
		printf(".");
	}
	printf("Fini.\n");
	pid1 = start(cons_rd0, 4000, 129, "cons_rd0", 0);
	pid2 = start(cons_rd0, 4000, 129, "cons_rd0", 0);
	pid3 = start(cons_rd0, 4000, 129, "cons_rd0", 0);
	pid4 = start(cons_rd0, 4000, 129, "cons_rd0", 0);
	waitpid(pid2, 0);
	waitpid(pid3, 0);
	waitpid(pid1, 0);
	waitpid(pid4, 0);

	printf("Enfin on teste que dans le cas d'entrees bloquantes, les processus sont servis\ndans l'ordre de leurs priorites. Entrez quatre lignes : ");
	pid1 = start(cons_rdN, 4000, 130, "cons_rdN", 0);
	pid2 = start(cons_rdN, 4000, 132, "cons_rdN", 0);
	pid3 = start(cons_rdN, 4000, 131, "cons_rdN", 0);
	pid4 = start(cons_rdN, 4000, 129, "cons_rdN", 0);
	waitpid(pid2, 0);
	waitpid(pid3, 0);
	waitpid(pid1, 0);
	waitpid(pid4, 0);
}

/*******************************************************************************
 * Test 20
 *
 * Le repas des philosophes.
 ******************************************************************************/
static char f[NR_PHILO]; /* tableau des fourchettes, contient soit 1 soit 0 selon si elle
			    est utilisee ou non */

static char bloque[NR_PHILO]; /* memorise l'etat du philosophe, contient 1 ou 0 selon que le philosophe
				 est en attente d'une fourchette ou non */

static struct sem mutex_philo; /* exclusion mutuelle */
static struct sem s[NR_PHILO]; /* un semaphore par philosophe */
static int etat[NR_PHILO];

static void
affiche_etat()
{
	int i;
	printf("%c", 13);
	for (i=0; i<NR_PHILO; i++) {
		unsigned long c;
		switch (etat[i]) {
		case 'm':
			c = 2;
			break;
		default:
			c = 4;
		}
		printf("%c", etat[i]);
	}
}

static void
waitloop(void)
{
	int j;
	for (j = 0; j < 5000; j++) {
		int l;
		test_it();
		for (l = 0; l < 5000; l++);
	}
}

static void
penser(long i)
{
	xwait(&mutex_philo); /* DEBUT SC */
	etat[i] = 'p';
	affiche_etat();
	xsignal(&mutex_philo); /* Fin SC */
	waitloop();
	xwait(&mutex_philo); /* DEBUT SC */
	etat[i] = '-';
	affiche_etat();
	xsignal(&mutex_philo); /* Fin SC */
}

static void
manger(long i)
{
	xwait(&mutex_philo); /* DEBUT SC */
	etat[i] = 'm';
	affiche_etat();
	xsignal(&mutex_philo); /* Fin SC */
	waitloop();
	xwait(&mutex_philo); /* DEBUT SC */
	etat[i] = '-';
	affiche_etat();
	xsignal(&mutex_philo); /* Fin SC */
}

static int
test(int i)
{
	/* les fourchettes du philosophe i sont elles libres ? */
	return ((!f[i] && (!f[(i + 1) % NR_PHILO])));
}

static void
prendre_fourchettes(int i)
{
	/* le philosophe i prend des fourchettes */

	xwait(&mutex_philo); /* Debut SC */

	if (test(i)) {  /* on tente de prendre 2 fourchette */
		f[i] = 1;
		f[(i + 1) % NR_PHILO] = 1;
		xsignal(&s[i]);
	} else
		bloque[i] = 1;

	xsignal(&mutex_philo); /* FIN SC */
	xwait(&s[i]); /* on attend au cas o on ne puisse pas prendre 2 fourchettes */
}

static void
poser_fourchettes(int i)
{

	xwait(&mutex_philo); /* DEBUT SC */

	if ((bloque[(i + NR_PHILO - 1) % NR_PHILO]) && (!f[(i + NR_PHILO - 1) % NR_PHILO])) {
		f[(i + NR_PHILO - 1) % NR_PHILO] = 1;
		bloque[(i + NR_PHILO - 1) % NR_PHILO] = 0;
		xsignal(&s[(i + NR_PHILO - 1) % NR_PHILO]);
	} else
		f[i] = 0;

	if ((bloque[(i + 1) % NR_PHILO]) && (!f[(i + 2) % NR_PHILO])) {
		f[(i + 2) % NR_PHILO] = 1;
		bloque[(i + 1) % NR_PHILO] = 0;
		xsignal(&s[(i + 1) % NR_PHILO]);
	} else
		f[(i + 1) % NR_PHILO] = 0;

	xsignal(&mutex_philo); /* Fin SC */
}

static int
philosophe(void *arg)
{
	/* comportement d'un seul philosophe */
	int i = (int) arg;
	int k;

	for (k = 0; k < 6; k++) {
		prendre_fourchettes(i); /* prend 2 fourchettes ou se bloque */
		manger(i); /* le philosophe mange */
		poser_fourchettes(i); /* pose 2 fourchettes */
		penser(i); /* le philosophe pense */
	}
	xwait(&mutex_philo); /* DEBUT SC */
	etat[i] = '-';
	affiche_etat();
	xsignal(&mutex_philo); /* Fin SC */
	return 0;
}

static int
launch_philo()
{

	int i, pid;

	for (i = 0; i < NR_PHILO; i++) {
		pid = start(philosophe, 4000, 192, "philosophe", (void *) i);
		assert(pid > 0);
	}
	return 0;

}

static void
test20(void)
{
	int j, pid;

	xscreate(&mutex_philo); /* semaphore d'exclusion mutuelle */
	xsignal(&mutex_philo);
	for (j = 0; j < NR_PHILO; j++) {
		xscreate(s + j); /* semaphore de bloquage des philosophes */
		f[j] = 0;
		bloque[j] = 0;
		etat[j] = '-';
	}

	printf("\n");
	pid = start(launch_philo, 4000, 193, "Lanceur philosophes", 0);
	assert(pid > 0);
	assert(waitpid(pid, 0) == pid);
	printf("\n");
	xsdelete(&mutex_philo);
	for (j = 0; j < NR_PHILO; j++) {
		xsdelete(s + j);
	}
}

/*******************************************************************************
 * Fin des tests
 ******************************************************************************/

static void auto_test(void);

static void
quit(void)
{
	exit(0);
}

static struct {
	const char *name;
	void (*f) (void);
} commands[] = {
	{"1", test1},
	{"2", test2},
	{"3", test3},
	{"4", test4},
	{"5", test5},
	{"6", test6},
	{"7", test7},
	{"8", test8},
	{"9", test9},
	{"10", test10},
	{"11", test11},
	{"12", test12},
	{"13", test13},
	{"14", test14},
	{"15", test15},
	{"16", test16},
	{"17", test17},
	{"18", test18},
	{"19", test19},
	{"20", test20},
	{"si", sys_info},
	{"a", auto_test},
	{"auto", auto_test},
	{"q", quit},
	{"quit", quit},
	{"exit", quit},
	{0, 0},
};

static void
auto_test(void)
{
	int i = 0;

	while (commands[i].f != sys_info) {
		printf("Test %s : ", commands[i].name);
		commands[i++].f();
	}
}

int
test_run(int n)
{
	assert(getprio(getpid()) == 128);
	if ((n < 1) || (n > 20)) {
		printf("%d: unknown test\n", n);
	} else {
		commands[n - 1].f();
	}
	return 0;
}

int
test_proc(void *arg)
{
	char buffer[20];

	assert(getprio(getpid()) == 128);

	unsigned long flags;
	unsigned long cs;
	unsigned long long seed;

	__asm__ volatile("pushfl; popl %0; movl %%cs,%1\n":"=r" (flags), "=r" (cs) ::"memory");
	printf("EFLAGS = %#lx, CS = %#lx\n", flags, cs);

	__asm__ __volatile__("rdtsc":"=A"(seed));
	setSeed(seed);

	while (1) {
		int i = 0;
		printf("Test (1-20, auto) : ");
		cons_gets(buffer, 20);
		while (commands[i].name && strcmp(commands[i].name, buffer)) i++;
		if (!commands[i].name) {
			printf("%s: unknown test\n", buffer);
		} else {
			commands[i].f();
		}
	}
}
