# PostgreSQL LDAP Authentication: A Step-by-Step Guide


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [PostgreSQL LDAP 认证的好处](#postgresql-ldap-认证的好处)
- <span class="section-num">2</span> [在 PostgreSQL 中配置 LDAP 身份验证](#在-postgresql-中配置-ldap-身份验证)
    - <span class="section-num">2.1</span> [修改 _pg_hba.conf_ 文件](#修改-pg-hba-dot-conf-文件)
    - <span class="section-num">2.2</span> [配置 PostgreSQL 服务器参数](#配置-postgresql-服务器参数)
    - <span class="section-num">2.3</span> [创建角色和授予权限](#创建角色和授予权限)
- <span class="section-num">3</span> [安全评分：PostgreSQL LDAP 身份验证安全吗？](#安全评分-postgresql-ldap-身份验证安全吗)
- <span class="section-num">4</span> [设置 PostgreSQL LDAP 身份验证的逐步指南](#设置-postgresql-ldap-身份验证的逐步指南)
- <span class="section-num">5</span> [结论](#结论)
- <span class="section-num">6</span> [常见问题解答](#常见问题解答)

</div>
<!--endtoc-->


本文为摘录(或转载)，侵删，原文为： https://goteleport.com/learn/postgresql-ldap-authentication/

PostgreSQL LDAP 认证允许您直接从现有的 LDAP 服务器（如 Active Directory 或 OpenLDAP）管理用户访问。这消除了单独的数据库用户账户的需要，并简化了用户管理。


## <span class="section-num">1</span> PostgreSQL LDAP 认证的好处 {#postgresql-ldap-认证的好处}

为 PostgreSQL 实施 LDAP 认证为您的组织带来了众多好处：

1.  **集中用户管理：** 从单一的 LDAP 目录管理所有数据库用户，而不是在 PostgreSQL 中创建和维护单独的凭证。这简化了新用户入职的过程，以及在必要时撤销访问权限。

2.  **增强安全性：** 利用您 LDAP 服务器现有的安全策略和控制，例如密码复杂性要求和多因素认证。

3.  **减少行政开销：** 自动化用户配置和撤销配置任务，使您的 IT 团队能专注于其他关键任务。

4.  **改善用户体验：** 用户可以使用熟悉的 LDAP 凭证访问 PostgreSQL 数据库，无需记住多个用户名和密码。


## <span class="section-num">2</span> 在 PostgreSQL 中配置 LDAP 身份验证 {#在-postgresql-中配置-ldap-身份验证}

让我们探讨如何为您的 PostgreSQL 数据库启用和配置 LDAP 身份验证。


### <span class="section-num">2.1</span> 修改 _pg_hba.conf_ 文件 {#修改-pg-hba-dot-conf-文件}

_pg_hba.conf_ 文件充当门卫，控制对您的 PostgreSQL 服务器的访问。您需要修改此配置文件以启用 LDAP 身份验证方法。

– 打开位于您的 PostgreSQL 数据目录中的 _pg_hba.conf_ 文件。– 添加新行，指定身份验证方法为 ldap，并包括必要的参数，例如您的 LDAP 服务器地址、端口和搜索基础。

例如：

```cfg
host    all             all             192.168.1.0/24          ldap ldapserver=ldap.example.com ldapport=389 ldapbinddn="cn=admin,dc=example,dc=com" ldapbindpasswd="password"
```


### <span class="section-num">2.2</span> 配置 PostgreSQL 服务器参数 {#配置-postgresql-服务器参数}

接下来，您需要在 PostgreSQL 服务器内部配置特定的参数。这包括：

– `ldapserver`: 指定您的 LDAP 服务器地址（例如， `ldapserver = 'ldap.example.com'` ）。– `ldapport`: 定义 LDAP 服务器端口（默认为 `389` ）。– `ldapbinddn`: （可选）指定用于绑定和搜索目录的 LDAP 用户。– `ldapbindpasswd`: （可选）ldapbinddn 用户的密码。– `ldapbasedn`: 设置 LDAP 搜索的基础 DN（例如， `ldapbasedn = 'dc=example,dc=com'` ）。– `ldapsearchattribute`: 确定用于匹配 PostgreSQL 用户名的属性（例如， `ldapsearchattribute = 'uid'` ）。


### <span class="section-num">2.3</span> 创建角色和授予权限 {#创建角色和授予权限}

– 以超级用户身份连接到您的 PostgreSQL 数据库。– 创建与您 LDAP 目录中的组或用户相对应的角色。例如： `CREATE ROLE developers WITH LOGIN;` 。– 使用标准 SQL  `GRANT` 语句向这些角色授予适当的权限。这使您能够控制每个 LDAP 用户或组可以访问哪些数据库对象。

通过遵循这些步骤，您成功地将 PostgreSQL LDAP 身份验证集成到您的环境中。这增强了您 PostgreSQL 数据库的安全性，并简化了您组织内的用户管理。


## <span class="section-num">3</span> 安全评分：PostgreSQL LDAP 身份验证安全吗？ {#安全评分-postgresql-ldap-身份验证安全吗}

评分：4分（满分 5 分）

从安全团队的角度来看，PostgreSQL LDAP 身份验证提供了一层强大的安全保障，尤其是与在本地管理数据库用户相比。利用 LDAP 服务器的集中认证机制、密码策略和潜在的多因素认证功能，显著增强了数据库的安全状态。

然而，像任何系统一样，它也有潜在的漏洞：

– LDAP 服务器被攻击：如果你的 LDAP 服务器被攻陷，攻击者可能获得对你的 PostgreSQL 数据库的访问权限。– 明文通信：标准 LDAP 通信是在明文中进行的，可能在传输过程中暴露凭证。考虑实施 LDAPS（SSL/TLS 上的 LDAP）或 StartTLS 以进行加密通信。


## <span class="section-num">4</span> 设置 PostgreSQL LDAP 身份验证的逐步指南 {#设置-postgresql-ldap-身份验证的逐步指南}

这里提供一个简化的指南，假设您有一个正常工作的 PostgreSQL 和 LDAP 环境：

1.  安装必要的软件包：确保你的 PostgreSQL 服务器安装了 LDAP 客户端库。软件包名称可能因操作系统而异（例如，Debian/Ubuntu 上的 postgresql-contrib）。
2.  修改 _pg_hba.conf_ ：编辑你的 _pg_hba.conf_ 文件（通常位于/var/lib/postgresql/data/）以包含 LDAP 身份验证的条目。以下是一个基本示例：
    ```cfg
    host    all             all             192.168.1.0/24          ldap ldapserver=ldap.example.com ldapport=389 ldapbinddn="cn=postgres_bind,ou=users,dc=example,dc=com" ldapbindpasswd="your_ldap_password" ldapbasedn="ou=users,dc=example,dc=com"
    ```

3.  配置 PostgreSQL 服务器参数：在 postgresql.conf 文件中添加或修改本文第 2 节中提到的参数（ldapserver、ldapport 等）。
4.  重启 PostgreSQL：重启你的 PostgreSQL 服务以应用更改。
5.  测试你的配置：尝试使用 LDAP 目录中存在的用户帐户登录，以确认 LDAP 身份验证是否正常工作。


## <span class="section-num">5</span> 结论 {#结论}

PostgreSQL LDAP 身份验证为寻求加强数据库安全和简化用户管理的组织提供了显著的优势。通过利用现有的 LDAP 基础设施，您简化了认证过程，减少了管理负担，并增强了 PostgreSQL 数据库的整体安全状态。


## <span class="section-num">6</span> 常见问题解答 {#常见问题解答}

-   我可以将 LDAP 身份验证与 Active Directory 结合使用吗？ <br />
    绝对可以！Active Directory 是 LDAP 协议的广泛使用实现。配置方式与其他 LDAP 服务器类似，只需确保使用正确的域详细信息和连接设置。

-   我是否可以强制从 LDAP 服务器对 PostgreSQL 用户实施密码复杂度规则？ <br />
    是的，当您实施 LDAP 身份验证时，PostgreSQL 会继承中央 LDAP 服务器强制的密码策略、复杂度要求和账户锁定设置。

-   如何排查 PostgreSQL 与 LDAP 服务器之间的 LDAP 连接问题？
    -   验证网络连接：确保你的 PostgreSQL 服务器能够通过网络访问你的 LDAP 服务器。使用=ping=或=telnet=等工具测试指定端口的连接性。

    -   检查 PostgreSQL 日志：检查 PostgreSQL 服务器日志中的任何与 LDAP 身份验证相关的错误消息。这些日志通常提供有关连接问题或配置错误的有价值的见解。

    -   使用调试工具：可以考虑使用 LDAP 调试工具来分析 PostgreSQL 服务器与 LDAP 服务器之间的通信流程，帮助定位任何问题的根本原因。

