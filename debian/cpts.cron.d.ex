#
# Regular cron jobs for the cpts package
#
0 4	* * *	root	[ -x /usr/bin/cpts_maintenance ] && /usr/bin/cpts_maintenance
