# Computer-network-FTP-project
# commit test

对于windows系统，如果使用vscode作为编辑器，那么需要Ctrl+shift+p创建tasks.json文件，并且做如下修改：

# 修改后

- ```json
    {
        "version": "2.0.0",
        "tasks": [
            {
                "type": "cppbuild",
                "label": "C/C++: gcc.exe 生成活动文件",
                "command": "C:\\MinGW\\bin\\gcc.exe",
                "args": [
                    "-fdiagnostics-color=always",
                    "-g",
                    "${file}",
                    "-o",
                    "${fileDirname -lwsock32}\\${fileBasenameNoExtension}.exe"
                ],
                "options": {
                    "cwd": "${fileDirname}"
                },
                "problemMatcher": [
                    "$gcc"
                ],
                "group": {
                    "kind": "build",
                    "isDefault": true
                },
                "detail": "编译器: C:\\MinGW\\bin\\gcc.exe"
            }
        ]
    }
    ```
