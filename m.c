/* GNU's users.
   Copyright (C) 1992-2024 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by jla; revised by djm */

#include <config.h>
#include <stdio.h>

#include <sys/types.h>
#include "system.h"

#include "long-options.h"
#include "quote.h"
#include "readutmp.h"

/* The official name of this program (e.g., no 'g' prefix).  */
#define PROGRAM_NAME "users"

#define AUTHORS \
  proper_name ("Joseph Arceneaux"), 
  proper_name ("David MacKenzie")

static int
userid_compare (const void *v_a, const void *v_b) //유저 아이디를 비교
{
  char **a = (char **) v_a;
  char **b = (char **) v_b;
  return strcmp (*a, *b);
}
//사용자 이름을 사전 순으로 정렬

static void // ????


// 구조체에 this 포인터???
list_entries_users (idx_t n, struct gl_utmp const *this) //utmp 엔트리는 사용자 이름, 터미널 이름, 호스트 이름, 로그인 시간 등
{
  char **u = xinmalloc (n, sizeof *u); //malloc과 유사하지만, 오류 처리를 자동으로 수행
  idx_t i;
  idx_t n_entries = 0;

  while (n--)
    {
      if (IS_USER_PROCESS (this))
        {
          char *trimmed_name;

          trimmed_name = extract_trimmed_name (this);

          u[n_entries] = trimmed_name;
          ++n_entries;
        }
      this++;
    }

  qsort (u, n_entries, sizeof (u[0]), userid_compare);

  for (i = 0; i < n_entries; i++)
    {
      char c = (i < n_entries - 1 ? ' ' : '\n');
      fputs (u[i], stdout);
      putchar (c);
    }

  for (i = 0; i < n_entries; i++)
    free (u[i]);
	  free (u);
}

/* Display a list of users on the system, according to utmp file FILENAME.
   Use read_utmp OPTIONS to read FILENAME.  */

//utmp 파일 처리 함수
static void
users (char const *filename, int options)
{
  idx_t n_users;
  struct gl_utmp *utmp_buf;
  options |= READ_UTMP_USER_PROCESS;
  if (read_utmp (filename, &n_users, &utmp_buf, options) != 0) //utmp 파일을 읽어들이는 함수
    error (EXIT_FAILURE, errno, "%s", quotef (filename));

  list_entries_users (n_users, utmp_buf);

  free (utmp_buf);
}


//사용법 출력 함수
void
usage (int status)
{
  if (status != EXIT_SUCCESS)  //상태가 올바르지 않다면 실패.
    emit_try_help ();
  else
    {
        // 프로그램 사용법의 첫 줄을 출력
      printf (_("Usage: %s [OPTION]... [FILE]\n"), program_name);
      //FILE로 사용되는 wtmp 파일 경로(WTMP_FILE)를 출력합니다.
      printf (_("\
Output who is currently logged in according to FILE.\n\
If FILE is not specified, use %s.  %s as FILE is common.\n\
\n\
"),
              UTMP_FILE, WTMP_FILE);
    //옵션에 대한 설명 출력
      fputs (HELP_OPTION_DESCRIPTION, stdout);
      fputs (VERSION_OPTION_DESCRIPTION, stdout);
      emit_ancillary_info (PROGRAM_NAME);
    }
  exit (status);
}

// 메인로직이 도는 곳
int
main (int argc, char **argv)
{
  initialize_main (&argc, &argv); //GNU Core 유틸에서 기본으로 제공하는 초기화 함수(프로그램의 전반적인 설정을 초기화)
  set_program_name (argv[0]); //프로그램 이름 설정(경로를 인자로 참조)
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR); //메세지 번역?
  textdomain (PACKAGE);

  atexit (close_stdout);

  parse_gnu_standard_options_only (argc, argv, PROGRAM_NAME, PACKAGE_NAME,
                                   Version, true, usage, AUTHORS,
                                   (char const *) nullptr);

  switch (argc - optind)
    {
    case 0:			/* users */  //인자가 없는 경우
      users (UTMP_FILE, READ_UTMP_CHECK_PIDS);
      break;

    case 1:			/* users <utmp file> */ //인자가 하나 있는 경우
      users (argv[optind], 0);
      break;

    default:			/* lose */
      error (0, 0, _("extra operand %s"), quote (argv[optind + 1])); //인자 두 개 이상인 경우
      usage (EXIT_FAILURE);
    }

  return EXIT_SUCCESS;
}