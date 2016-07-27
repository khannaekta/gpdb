#!/bin/bash -l

set -exo pipefail

function gen_env(){
	cat > /home/gpadmin/run_make.sh <<-EOF
	source /opt/gcc_env.sh
	ln -s "$(pwd)/gpdb_src/gpAux/ext/rhel5_x86_64/python-2.6.2" /opt
	source /home/gpadmin/greenplum-db-devel/greenplum_path.sh

	sh /home/gpadmin/setup_db.sh
	cd "\${1}/gpdb_src/gpAux/extensions/gps3ext"
	make -B install

	cd regress
	# Replace path to use compiled pg_regress which exists in docker image
	sed -i -e 's/\$(shell .*/\/home\/gpadmin\/workspace\/gpdb4/g' Makefile
	make installcheck
	EOF

	chown -R gpadmin:gpadmin $(pwd)
	chown gpadmin:gpadmin /home/gpadmin/run_make.sh
	chmod a+x /home/gpadmin/run_make.sh
}


function _main() {
	gen_env

	# See https://gist.github.com/gasi/5691565
	sed -ri 's/UsePAM yes/UsePAM no/g' /etc/ssh/sshd_config
	# Disable password authentication so builds never hang given bad keys
	sed -ri 's/PasswordAuthentication yes/PasswordAuthentication no/g' /etc/ssh/sshd_config
	/etc/init.d/sshd start

	chmod u+s `which ping`

	echo -n "$s3conf" >/home/gpadmin/s3.b64
	base64 -d /home/gpadmin/s3.b64 >/home/gpadmin/s3.conf
	chown gpadmin:gpadmin /home/gpadmin/s3.conf
	cat /home/gpadmin/s3.conf

	su - gpadmin -c "bash /home/gpadmin/run_make.sh $(pwd)"
}

_main "$@"
