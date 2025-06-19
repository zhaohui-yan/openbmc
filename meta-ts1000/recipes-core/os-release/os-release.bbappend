# FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

# python do_compile:prepend() {
#     import re
#     work_dir = d.getVar("COREBASE")
#     sdk_ver = ""
#     try:
#         res=bb.process.run(("git -C %s symbolic-ref -q --short HEAD ||" +
#                             "git -C %s describe --tags --exact-match")
#             % (work_dir, work_dir))[0].strip("\n")

#         tag_ver = re.search("(v\d+\.\d+)", res)
#         if tag_ver:
#             sdk_ver=tag_ver.group(1)
#         else:
#             sdk_ver=bb.process.run("git -C %s rev-parse HEAD"
#                 % work_dir)[0].strip("\n")
#     except Exception as e:
#         print(e)
#         bb.warn("Failed to get SDK version")
#         pass
#     d.setVar("SDK_VERSION", sdk_ver)
# }

# # VERSION_ID = "${VERSION}"


# OS_RELEASE_FIELDS:append = " SDK_VERSION"
# BB_DONT_CACHE = "1"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

# 重写 VERSION_ID 定义
VERSION_ID = "${@get_extended_version(d)}"

# 定义获取版本号的函数
def get_extended_version(d):
    import re
    import subprocess
    work_dir = d.getVar("COREBASE")
    extended_ver = "unknown"  # 默认值

    try:
        # 获取基础标签 (如 v09.06)
        base_tag_cmd = ["git", "-C", work_dir, "describe", "--tags", "--abbrev=0"]
        base_tag = subprocess.check_output(base_tag_cmd, text=True).strip()

        # 获取距离标签的提交次数
        count_cmd = ["git", "-C", work_dir, "rev-list", "--count", f"{base_tag}..HEAD"]
        commit_count = subprocess.check_output(count_cmd, text=True).strip()

        # 构建目标格式 v09.06-1491
        extended_ver = f"{base_tag}-{commit_count}"

    except Exception as e:
        bb.warn("Primary version method failed: %s" % str(e))
        try:
            # 降级方案：使用标准describe并提取
            desc_cmd = ["git", "-C", work_dir, "describe", "--tags", "--long"]
            res = subprocess.check_output(desc_cmd, text=True).strip()

            # 正则提取 vXX.XX-XXXX 格式 (忽略哈希和dirty标记)
            match = re.search(r"(v\d+\.\d+-\d+)", res)
            if match:
                extended_ver = match.group(1)
            else:
                # 备用方案：取前两部分 (v09.06-1491)
                parts = res.split('-')
                extended_ver = '-'.join(parts[:2]) if len(parts) >= 2 else res
        except Exception as fallback_e:
            bb.warn("Fallback version method failed: %s" % str(fallback_e))

    return extended_ver

# 移除旧的 SDK_VERSION 逻辑
BB_DONT_CACHE = "1"
