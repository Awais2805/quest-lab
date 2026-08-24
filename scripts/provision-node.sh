#!/usr/bin/env bash
# Compute node provisioning. Idempotent - safe to re-run.
set -euo pipefail
exec > >(tee -a /var/log/provision.log) 2>&1
echo "=== provision $(date -Is) ==="

HEAD_IP=${HEAD_IP:-10.0.0.4}
CLUSTER_USER=${CLUSTER_USER:-hpcuser}
CLUSTER_UID=${CLUSTER_UID:-11000}
MPI_SUBNET=${MPI_SUBNET:-10.0.1.0/24}

# --- fixed service UIDs, before the packages land -------------------------
# Debian allocates these dynamically, giving a different UID per node. Munge
# credentials and Slurm RPCs both carry uid/gid, so a mismatch reads as an
# auth failure that looks exactly like a network failure.
for spec in munge:1101 slurm:1102; do
  n=${spec%%:*}; u=${spec##*:}
  getent group "$n" >/dev/null || groupadd -g "$u" "$n"
  getent passwd "$n" >/dev/null || \
    useradd -u "$u" -g "$n" -d /nonexistent -s /usr/sbin/nologin -M "$n"
done

# --- packages -------------------------------------------------------------
export DEBIAN_FRONTEND=noninteractive
apt-get update -y
apt-get install -y --no-install-recommends \
  build-essential cmake git \
  openmpi-bin libopenmpi-dev \
  munge slurmd slurm-client \
  nfs-common jq numactl hwloc curl

# --- cluster user, same UID everywhere ------------------------------------
getent group "$CLUSTER_USER" >/dev/null || groupadd -g "$CLUSTER_UID" "$CLUSTER_USER"
getent passwd "$CLUSTER_USER" >/dev/null || \
  useradd -u "$CLUSTER_UID" -g "$CLUSTER_USER" -M \
          -d /shared/home/"$CLUSTER_USER" -s /bin/bash "$CLUSTER_USER"

# --- dual-NIC sanity ------------------------------------------------------
# Loose reverse-path filtering: strict mode drops legitimate packets on
# multi-homed hosts where replies leave via a different NIC than they arrived.
cat > /etc/sysctl.d/90-cluster.conf <<'EOF'
net.ipv4.conf.all.rp_filter = 2
net.ipv4.conf.default.rp_filter = 2
EOF
sysctl -p /etc/sysctl.d/90-cluster.conf

# Pin MPI to the cluster NIC system-wide, so no mpirun invocation has to
# remember --mca flags. With two NICs OpenMPI otherwise picks one at random
# or tries both and stalls.
mkdir -p /etc/openmpi
cat > /etc/openmpi/openmpi-mca-params.conf <<EOF
btl_tcp_if_include = ${MPI_SUBNET}
oob_tcp_if_include = ${MPI_SUBNET}
EOF

# --- shared filesystem ----------------------------------------------------
mkdir -p /shared
grep -q ' /shared ' /etc/fstab || \
  echo "${HEAD_IP}:/shared /shared nfs4 _netdev,hard,timeo=600,retrans=2,nofail 0 0" >> /etc/fstab
for i in $(seq 1 30); do
  mountpoint -q /shared && break
  mount /shared 2>/dev/null || sleep 10
done

# --- slurm, configless ----------------------------------------------------
# slurmd pulls slurm.conf from slurmctld at startup: nothing to distribute,
# and the boot path does not depend on NFS.
install -d -m 0755 -o slurm -g slurm /var/log/slurm
install -d -m 0755 /var/spool/slurmd
echo "SLURMD_OPTIONS=\"--conf-server ${HEAD_IP}:6817\"" > /etc/default/slurmd
systemctl enable slurmd

# --- record exactly what this node is, for the lab notebook ---------------
{
  echo "provisioned: $(date -Is)"
  echo "kernel:      $(uname -r)"
  echo "cpu:         $(lscpu | awk -F: '/Model name/{gsub(/^ +/,"",$2); print $2}')"
  echo "vcpus:       $(nproc)"
  echo "memory:      $(free -g | awk '/^Mem:/{print $2} ') GiB"
  echo "gcc:         $(gcc --version | head -1)"
  echo "openmpi:     $(mpirun --version | head -1)"
  echo "slurmd:      $(slurmd --version 2>/dev/null || echo n/a)"
} > /etc/cluster-provenance
cat /etc/cluster-provenance
echo "=== provision complete ==="
