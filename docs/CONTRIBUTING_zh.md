# 贡献指南

感谢您对 userd 项目的关注！本文档将帮助您了解如何为项目做出贡献。

## 目录

- [如何贡献](#如何贡献)
- [开发环境设置](#开发环境设置)
- [代码规范](#代码规范)
- [提交规范](#提交规范)
- [Pull Request 流程](#pull-request-流程)

## 如何贡献

### 报告 Bug

如果您发现了 Bug，请通过 GitHub Issues 提交报告，并包含以下信息：

1. 问题的详细描述
2. 复现步骤
3. 期望行为和实际行为
4. 操作系统和编译器版本
5. 相关的配置文件或日志

### 提交功能建议

功能建议也请通过 GitHub Issues 提交，请包含：

1. 功能的详细描述
2. 使用场景
3. 期望的实现方式

### 提交代码

1. Fork 本仓库
2. 创建您的特性分支 (`git checkout -b feature/your-feature`)
3. 提交您的修改 (`git commit -m 'Add some feature'`)
4. 推送到分支 (`git push origin feature/your-feature`)
5. 创建一个 Pull Request

## 开发环境设置

### 依赖要求

- C++17 或更高版本
- POSIX 兼容系统（Linux、macOS）
- g++ 或 clang++ 编译器

### 构建项目

```bash
# 克隆仓库
git clone https://github.com/Carry-Rao/userd.git
cd userd

# 构建调试版本
make debug
```

### 运行测试

```bash
# 运行所有测试（如果存在）
make test
```

## 代码规范

### C++ 代码风格

1. 遵循 [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
2. 使用 4 个空格进行缩进，不使用 Tab
3. 行宽限制在 100 个字符以内
4. 使用 `snake_case` 命名变量和函数
5. 使用 `PascalCase` 命名类和结构体
6. 使用 `UPPER_CASE` 命名常量和宏

### 文件组织

1. 头文件使用 `.hpp` 扩展名
2. 源文件使用 `.cpp` 扩展名
3. 每个类或结构体应该在单独的头文件中定义
4. 使用 `#pragma once` 防止头文件重复包含

### 命名约定

1. 类名：`PascalCase`
2. 函数名：`snake_case`
3. 变量名：`snake_case`
4. 常量名：`UPPER_CASE`
5. 成员变量：使用下划线前缀 `_variable`

### 注释规范

1. 所有公共接口必须有文档注释
2. 复杂的算法或逻辑需要添加注释
3. 使用 `//` 进行行注释
4. 使用 `/* */` 进行块注释

## 提交规范

提交信息应该遵循以下格式：

```
类型: 简短描述

详细描述（可选）
```

### 类型

- `Feature`：新功能
- `Fix`：Bug 修复
- `Docs`：文档更新
- `Style`：代码格式调整（不影响代码运行）
- `Refactor`：代码重构
- `Test`：添加或修改测试
- `Chore`：构建过程或辅助工具的变动

### 示例

```
Feature: 添加服务状态查询功能

支持通过 userctl status 命令查询服务运行状态
```

```
Fix: 修复服务重启失败的问题

修复了在服务异常退出后无法正确重启的问题
```

## Pull Request 流程

1. **Fork 仓库**：在 GitHub 上 Fork 本仓库
2. **创建分支**：从 `main` 分支创建您的特性分支
3. **编写代码**：按照代码规范编写代码
4. **添加测试**：为新功能或 Bug 修复添加测试
5. **更新文档**：更新相关的文档和注释
6. **提交代码**：按照提交规范提交代码
7. **创建 PR**：在 GitHub 上创建 Pull Request
8. **代码审查**：等待项目维护者进行代码审查
9. **合并代码**：审查通过后，代码将被合并到主分支

### PR 描述模板

```markdown
## 描述

简要描述您的更改

## 更改类型

- [ ] Bug 修复
- [ ] 新功能
- [ ] 文档更新
- [ ] 代码重构
- [ ] 其他

## 测试

描述您如何测试这些更改

## 相关 Issue

关闭 #issue_number
```

## 获取帮助

如果您有任何问题或需要帮助，请通过以下方式联系：

- 创建 GitHub Issue
- 发送邮件至项目维护者

感谢您的贡献！