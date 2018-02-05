#!/usr/bin/zsh
echo "Copiant a OVH"
scp -i /mnt/ntfs/mx300/SSHKEYS/id_rsa.key ~/CLionProjects/PLUMI-xarxes/*.c ~/CLionProjects/PLUMI-xarxes/*.h root@ovh.ledgedash.com:/opt/xarxes/
echo "Compilant a OVH"
ssh -i /mnt/ntfs/mx300/SSHKEYS/id_rsa.key root@ovh.ledgedash.com "cd /opt/xarxes;gcc MIp2-lumi.c MIp2-mi.c MIp2-nodelumi.c -o servidor -w -std=c11;gcc MIp2-lumi.c MIp2-mi.c MIp2-p2p.c -o client -w -std=c11;" 
echo "Copiant a Aruba"
scp -i /mnt/ntfs/mx300/SSHKEYS/id_rsa.key ~/CLionProjects/PLUMI-xarxes/*.c ~/CLionProjects/PLUMI-xarxes/*.h root@aruba.ledgedash.com:/opt/xarxes/
echo "Compilant a Aruba"
ssh -i /mnt/ntfs/mx300/SSHKEYS/id_rsa.key root@Aruba.ledgedash.com "cd /opt/xarxes;gcc MIp2-lumi.c MIp2-mi.c MIp2-nodelumi.c -o servidor -w -std=c11;gcc MIp2-lumi.c MIp2-mi.c MIp2-p2p.c -o client -w -std=c11;" 