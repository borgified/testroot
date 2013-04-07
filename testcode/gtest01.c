/**
 * gtest01.c - miscellaneous glib mainloop-based tests.
 * Most of these tests involve the glib mainloop - which is harder to test in python.
 * Otherwise we prefer to test in python because it exercises both the C code and the python wrappers for it
 * and for the most part, they're easier to write.
 *
 * This file is part of the Assimilation Project.
 *
 * @author  Alan Robertson <alanr@unix.sh> - Copyright &copy; 2013 - Assimilation Systems Limited
 * @n
 *  The Assimilation software is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The Assimilation software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the Assimilation Project software.  If not, see http://www.gnu.org/licenses/
 */

#include <projectcommon.h>
#include <string.h>
#include <glib.h>
#include <gmainfd.h>
#include <logsourcefd.h>


GMainLoop*	mainloop;
FSTATIC void	test_read_command_output_at_EOF(void);
FSTATIC void	test_log_command_output(void);
FSTATIC void	quit_at_child_exit(GPid pid, gint status, gpointer gmainfd);
FSTATIC void	check_output_at_exit(GPid pid, gint status, gpointer gmainfd);

#define	HELLOSTRING	"Hello, world."

/// Make sure we read our HELLOSTRING when the process exits
FSTATIC void
check_output_at_exit(GPid pid, gint status, gpointer gmainfd)
{
	GMainFd*	cmdout = CASTTOCLASS(GMainFd, gmainfd);
	(void)pid;
	g_assert_cmpint(status, ==, 0);
	g_assert(cmdout->textread != NULL);
	g_assert_cmpint(strnlen(cmdout->textread->str, cmdout->textread->len), ==, cmdout->textread->len);
	g_assert_cmpstr(cmdout->textread->str, ==, HELLOSTRING "\n");
	//g_print("GOT [%s]\n", cmdout->textread->str);
	g_main_loop_quit(mainloop);
}

/// Test to run a command and verify that we can capture its output in a string in the gmainloop environment
FSTATIC void
test_read_command_output_at_EOF(void)
{
	GPid		childpid;
	gint		stdoutfd;
	GError*		failcode = NULL;
	GMainFd*	cmdout;
	gchar		echo[] = "/bin/echo";
	gchar		hello[] = HELLOSTRING;
	gchar* 	argv[] = {echo, hello, NULL};		// Broken glib API...
	gint	cmdid;
	g_assert(TRUE);
	mainloop = g_main_loop_new(g_main_context_default(), TRUE);
	g_spawn_async_with_pipes(
		NULL,				// Current directory
		argv,				// Arguments
		NULL,				// environment
		G_SPAWN_DO_NOT_REAP_CHILD,	// GSpawnFlags flags,
		NULL,				// GSpawnChildSetupFunc child_setup,
		NULL,				// gpointer user_data,
		&childpid,			// GPid *child_pid,
		NULL,				// gint *standard_input,
		&stdoutfd,			// gint *standard_output,
		NULL,				// gint *standard_error,
		&failcode);			// GError **error
	
	cmdout = gmainfd_new(0, stdoutfd, G_PRIORITY_HIGH, g_main_context_default());
	//g_print("Spawned child %d with fd %d\n", childpid, stdoutfd);
	g_assert_cmpint(childpid, >, 0);
	g_assert_cmpint(stdoutfd, >, 0);
	cmdid = g_child_watch_add(childpid, check_output_at_exit, cmdout);
	g_assert_cmpint(cmdid, >, 0);
	g_main_loop_run(mainloop);
	// We may be making an unnecessary ref call in the class implementation...
	g_source_unref(&cmdout->baseclass);
	g_source_unref(&cmdout->baseclass);
	g_main_loop_unref(mainloop);
	mainloop=NULL;
	if (proj_class_live_object_count() != 0) {
		proj_class_dump_live_objects();
	}
	g_assert_cmpint(proj_class_live_object_count(), ==, 0);
}

FSTATIC void
quit_at_child_exit(GPid pid, gint status, gpointer logsourcefd)
{
	LogSourceFd*	logsrc = CASTTOCLASS(LogSourceFd, logsourcefd);
	
	(void)pid;
	g_assert_cmpint(status, ==, 0);
	g_assert_cmpint(logsrc->linecount, ==, 1);
	// sizeof(HELLOSTRING) includes a NULL.  echo effectively replaces the NULL with a '\n'
	g_assert_cmpint(logsrc->charcount, ==, sizeof(HELLOSTRING));
	g_main_loop_quit(mainloop);
}

FSTATIC void
test_log_command_output(void)
{
	GPid		childpid;
	gint		stdoutfd;
	GError*		failcode = NULL;
	LogSourceFd*	cmdlog;
	gchar		echo[] = "/bin/echo";
	gchar		hello[] = HELLOSTRING;
	gchar* 	argv[] = {echo, hello, NULL};		// Broken glib API...
	gint	cmdid;
	g_assert(TRUE);
	mainloop = g_main_loop_new(g_main_context_default(), TRUE);
	g_spawn_async_with_pipes(
		NULL,				// Current directory
		argv,				// Arguments
		NULL,				// environment
		G_SPAWN_DO_NOT_REAP_CHILD,	// GSpawnFlags flags,
		NULL,				// GSpawnChildSetupFunc child_setup,
		NULL,				// gpointer user_data,
		&childpid,			// GPid *child_pid,
		NULL,				// gint *standard_input,
		&stdoutfd,			// gint *standard_output,
		NULL,				// gint *standard_error,
		&failcode);			// GError **error
	
	cmdlog = logsourcefd_new(0, stdoutfd, G_PRIORITY_HIGH, g_main_context_default()
	,			 G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, "TEST: ");
	//g_print("Spawned child %d with fd %d\n", childpid, stdoutfd);
	g_assert_cmpint(childpid, >, 0);
	g_assert_cmpint(stdoutfd, >, 0);
	cmdid = g_child_watch_add(childpid, quit_at_child_exit, cmdlog);
	g_assert_cmpint(cmdid, >, 0);
	g_main_loop_run(mainloop);
	// The implementation seems to have an extra ref() in it...
	g_source_unref(&cmdlog->baseclass.baseclass);
	g_source_unref(&cmdlog->baseclass.baseclass);
	g_main_loop_unref(mainloop);
	mainloop=NULL;
	if (proj_class_live_object_count() != 0) {
		proj_class_dump_live_objects();
	}
	g_assert_cmpint(proj_class_live_object_count(), ==, 0);
}

/// Test main program ('/gmain') using the glib test fixtures
int
main(int argc, char ** argv)
{
	g_setenv("G_MESSAGES_DEBUG", "all", TRUE);
	g_test_init(&argc, &argv, NULL);
	g_test_add_func("/gtest01/gmain/command-output", test_read_command_output_at_EOF);
	g_test_add_func("/gtest01/gmain/log-command-output", test_log_command_output);
	return g_test_run();
}
