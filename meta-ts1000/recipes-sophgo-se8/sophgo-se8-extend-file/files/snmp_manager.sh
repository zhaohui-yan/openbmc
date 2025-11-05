#!/bin/bash

CONFIG_FILE="/etc/snmp/snmpd.conf"

# 获取下一个view名称
get_next_view_name() {
    last=$(awk '/^view[ \t]+vw[0-9]+/ { sub(/^view[ \t]+vw/, "", $0); n=$1; if(n>max) max=n } END { print max }' "$CONFIG_FILE")
    if [ -z "$last" ]; then
        echo "vw1"
    else
        next=$((last+1))
        echo "vw$next"
    fi
}

# 增加
add_entry() {
    community="$1"
    oid="$2"
    view=$(get_next_view_name)

    # 检查community是否已存在
    if grep -q "rocommunity $community " "$CONFIG_FILE"; then
        echo "Community $community already exists."
        exit 1
    fi
    last_rc_line=$(grep -n '^rocommunity ' "$CONFIG_FILE" | tail -1 | cut -d: -f1)

    # 增加到权限配置文件的末尾
    sed -i "$((last_rc_line+1))a\\" "$CONFIG_FILE"
    sed -i "$((last_rc_line+2))i\rocommunity $community default -V $view" "$CONFIG_FILE"
    sed -i "$((last_rc_line+2))i\view $view included $oid 0xff" "$CONFIG_FILE"
    echo "Added: view $view included $oid 0xff"
    echo "Added: rocommunity $community default -V $view"
}

# 删除
delete_entry() {
    community="$1"
    # 查找对应view名
    view=$(grep "rocommunity $community " "$CONFIG_FILE" | awk '{for(i=1;i<=NF;i++) if($i=="-V") print $(i+1)}')
    if [ -z "$view" ]; then
        echo "Community $community not found."
        exit 1
    fi

    # 删除view和rocommunity行
    sed -i "/view $view included /d" "$CONFIG_FILE"
    sed -i "/rocommunity $community /d" "$CONFIG_FILE"
    echo "Deleted: view $view & rocommunity $community"
}

# 修改
modify_entry() {
    old_community="$1"
    old_oid="$2"
    new_community="$3"
    new_oid="$4"

    # 查找对应view名
    view=$(grep "rocommunity $old_community " "$CONFIG_FILE" | awk '{for(i=1;i<=NF;i++) if($i=="-V") print $(i+1)}')
    if [ -z "$view" ]; then
        echo "\"$old_community\" not found."
        exit 1
    fi
    oid=$(grep "view $view included $old_oid 0xff" "$CONFIG_FILE" | awk '{for(i=1;i<=NF;i++) if($i=="included") print $(i+1)}')
    if [ -z "$oid" ]; then
        echo "\"$old_community\" and \"$old_oid\" is not match."
        exit 1
    fi
    # 修改view行
    sed -i "s|^view $view included $old_oid 0xff|view $view included $new_oid 0xff|" "$CONFIG_FILE"
    # 修改rocommunity行
    sed -i "s|^rocommunity $old_community default -V $view|rocommunity $new_community default -V $view|" "$CONFIG_FILE"
    echo "Modified: view $view and rocommunity $new_community"
}

# 查询
query_entries() {
    result="["
    while read -r rc_line; do
        # rocommunity enterprise default -V vw1
        community=$(echo "$rc_line" | awk '{print $2}')
        view=$(echo "$rc_line" | awk '{for(i=1;i<=NF;i++) if($i=="-V") print $(i+1)}')
        # 查找对应的view行
        oid=$(grep "^view $view included " "$CONFIG_FILE" | awk '{print $4}')
        if [[ -n "$community" && -n "$oid" ]]; then
            result="${result}($community, $oid), "
        fi
    done < <(grep '^rocommunity ' "$CONFIG_FILE")
    result="${result%, }]"
    echo "$result"
}

# 主逻辑
case "$1" in
    add)
        if [ $# -ne 3 ]; then
            echo "用法: $0 add <community> <oid>"
            exit 1
        fi
        add_entry "$2" "$3"
        ;;
    rm)
        if [ $# -ne 2 ]; then
            echo "用法: $0 rm <community>"
            exit 1
        fi
        delete_entry "$2"
        ;;
    set)
        if [ $# -ne 5 ]; then
            echo "用法: $0 set <old_community> <old_oid> <new_community> <new_oid>"
            exit 1
        fi
        modify_entry "$2" "$3" "$4" "$5"
        ;;
    get)
        query_entries
        ;;
    *)
        echo "用法: $0 {add|rm|set|get} ..."
        exit 1
        ;;
esac
