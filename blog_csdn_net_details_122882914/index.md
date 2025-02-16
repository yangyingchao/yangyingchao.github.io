# 微软软件安全开发流程（SDL） security-development-lifecycle


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Microsoft 安全开发生命周期(SDL)](#microsoft-安全开发生命周期--sdl)
    - <span class="section-num">1.1</span> [培训](#培训)
    - <span class="section-num">1.2</span> [要求](#要求)
    - <span class="section-num">1.3</span> [Design](#design)
    - <span class="section-num">1.4</span> [实现](#实现)
    - <span class="section-num">1.5</span> [验证](#验证)
    - <span class="section-num">1.6</span> [发布](#发布)
    - <span class="section-num">1.7</span> [响应](#响应)
- <span class="section-num">2</span> [微软 SDL 流程终极整理总结](#微软-sdl-流程终极整理总结)
    - <span class="section-num">2.1</span> [一、微软 SDL(Security Development Lifecycle)流程框架](#一-微软-sdl--security-development-lifecycle--流程框架)
    - <span class="section-num">2.2</span> [二、主要步骤](#二-主要步骤)
    - <span class="section-num">2.3</span> [三、SDL 实战经验准则](#三-sdl-实战经验准则)
    - <span class="section-num">2.4</span> [四、与 SAMM 对比](#四-与-samm-对比)
    - <span class="section-num">2.5</span> [五、常用 SDL 实施方法和工具](#五-常用-sdl-实施方法和工具)
    - <span class="section-num">2.6</span> [六、敏捷 SDL](#六-敏捷-sdl)
    - <span class="section-num">2.7</span> [七、软件安全开发流程新增的安全活动](#七-软件安全开发流程新增的安全活动)
    - <span class="section-num">2.8</span> [八、附件《安全选项表》](#八-附件-安全选项表)

</div>
<!--endtoc-->


本文为摘录，原文为： https://blog.csdn.net/weixin_43965597/article/details/122882914



## <span class="section-num">1</span> Microsoft 安全开发生命周期(SDL) {#microsoft-安全开发生命周期--sdl}

<https://learn.microsoft.com/zh-cn/compliance/assurance/assurance-microsoft-security-development-lifecycle>

在开发安全软件时，安全和隐私绝不能事后考虑，必须制定正式流程，以确保在产品生命周期
的所有阶段都考虑它们。 Microsoft 的安全开发生命周期 (SDL) 将所有软件产品的开发和操
作中嵌入了全面的安全要求、技术特定工具和强制性流程。 Microsoft 的所有开发团队都必须
遵守 SDL 流程和要求，从而生成更安全的软件，减少和减少严重的漏洞，同时降低开发成本。

{{< figure src="/ox-hugo/assurance-sdl-process-diagram.png" >}}

Microsoft SDL 由七个组件组成，包括五个核心阶段和两个支持安全活动。 五个核心阶段是要
求、设计、实现、验证和发布。 其中每个阶段都包含强制性检查和批准，以确保正确满足所有
安全和隐私要求以及最佳做法。 两个支持的安全活动（培训和响应）分别在核心阶段之前和之
后进行，以确保它们得到正确实施，并且软件在部署后保持安全。


### <span class="section-num">1.1</span> 培训 {#培训}

所有 Microsoft 员工都必须完成一般安全和隐私意识培训，以及与其角色相关的特定培训。 新员工在受聘时会提供初始培训，
在 Microsoft 的整个就业过程中，需要每年进行一次进修培训。

开发人员和工程师还必须参加角色特定培训，以便随时了解安全基础知识和安全开发的最新趋势。 还鼓励所有全职员工、实习
生、临时员工、分包商和第三方寻求高级安全和隐私培训。


### <span class="section-num">1.2</span> 要求 {#要求}

Microsoft 开发的每个产品、服务和功能都从明确定义的安全和隐私要求开始;它们是安全应用
程序的基础，并告知其设计。 开发团队根据产品将处理的数据类型、已知威胁、最佳做法、法
规和行业要求以及从以前事件中吸取的经验教训等因素来定义这些要求。 定义后，将明确记录
和跟踪要求。

软件开发是一个持续的过程，这意味着相关的安全和隐私要求在产品的整个生命周期中都会发
生变化，以反映功能和威胁环境的变化。


### <span class="section-num">1.3</span> Design {#design}

定义安全、隐私和功能要求后，可以开始设计软件。 作为设计过程的一部分，会创建威胁模型，
以帮助根据风险识别、分类潜在威胁并对其进行评分。 在对软件进行更改时，必须在每个产品
的整个生命周期内维护和更新威胁模型。

{{< figure src="https://learn.microsoft.com/zh-cn/compliance/media/assurance-threat-modeling-diagram.png" >}}

{{< figure src="/ox-hugo/assurance-threat-modeling-diagram.png" >}}

{{< figure src="/ox-hugo/assurance-threat-modeling-diagram.png" >}}

威胁建模过程首先定义产品的不同组件，以及它们在关键功能方案中（如身份验证）中彼此交
互的方式。 数据流创建 (DFD) 的关系图，以直观地表示使用的关键数据流交互、数据类型、
端口和协议。 DFD 用于识别威胁并确定威胁的优先级，以便针对添加到产品安全要求的缓解措
施。

服务团队使用 Microsoft Threat Modeling Tool 创建威胁模型，使团队能够：

-   就其系统的安全设计进行沟通
-   使用经过验证的方法分析安全设计中的潜在安全问题
-   建议和管理安全问题的缓解措施

在任何产品发布之前，都会检查所有威胁模型的准确性和完整性，包括缓解不可接受的风险。


### <span class="section-num">1.4</span> 实现 {#实现}

实现从开发人员根据他们在前两个阶段创建的计划编写代码开始。 Microsoft 为开发人员提供
了一套安全开发工具，可有效实现他们设计的软件的所有安全性、隐私和功能要求。 这些工具
包括编译器、安全开发环境和内置安全检查。


### <span class="section-num">1.5</span> 验证 {#验证}

在发布任何书面代码之前，需要进行多次检查和批准，以验证代码是否符合 SDL、满足设计要
求以及是否没有编码错误。 手动评审由审阅者与开发代码的工程师分开进行。 职责分离是此
步骤中的一项重要控制措施，可最大程度地降低代码被编写和释放导致意外或恶意伤害的风险。

还需要各种自动检查，并内置在管道中，以在检查期间和编译生成时分析代码。 Microsoft 使
用的安全检查分为以下类别：

-   静态代码分析：分析源代码是否存在潜在的安全漏洞，包括代码中是否存在凭据。
-   二进制分析：在二进制代码级别评估漏洞，以确认代码已准备好生产。
-   凭据和机密扫描程序：识别源代码和配置文件中凭据和机密公开的可能实例。
-   加密扫描：验证源代码和代码执行中的加密最佳做法。
-   模糊测试：使用格式不正确和意外的数据来练习 API 和分析程序，以检查漏洞并验证错误处
    理。
-   配置验证：根据安全标准和最佳做法分析生产系统的配置。
-   组件治理 (CG) ：开源软件检测和版本、漏洞和法律义务检查。

如果手动审阅者或自动化工具发现代码存在任何问题，则系统会通知提交者，要求他们进行必
要的更改，然后再提交代码以供审阅。

此外，内部和外部提供商定期在 Microsoft 联机服务上进行渗透测试。 渗透测试提供了另一
种方法来发现其他方法未检测到的安全漏洞。 若要详细了解 Microsoft 的渗透测试，请参阅
Microsoft 365 中的攻击模拟。


### <span class="section-num">1.6</span> 发布 {#发布}

通过所有必需的安全测试和评审后，不会立即向所有客户发布版本。 在 SDP () 的安全部署流
程中，生成将系统地逐步发布到越来越大的组（称为环）。 SDP 环通常可定义为：

-   圈 0：负责服务或功能的开发团队
-   圈 1：所有 Microsoft 员工
-   圈 2：Microsoft 外部的用户，他们已将其组织或特定用户配置为使用目标发布通道
-   环 3：子阶段的全球标准发布

除了环 3，生成在每个环中保留适当天数，但高负载周期除外，因为版本已在早期环中进行了
适当的稳定性测试。


### <span class="section-num">1.7</span> 响应 {#响应}

所有 Microsoft 服务在发布后都会进行广泛记录和监视，使用集中式专有近实时监视系统识别
潜在的安全事件。 若要详细了解 Microsoft 的安全监视和安全事件管理，请参阅 安全监视概
述 和 Microsoft 安全事件管理。


## <span class="section-num">2</span> 微软 SDL 流程终极整理总结 {#微软-sdl-流程终极整理总结}

<https://blog.csdn.net/weixin_43965597/article/details/122882914>


### <span class="section-num">2.1</span> 一、微软 SDL(Security Development Lifecycle)流程框架 {#一-微软-sdl--security-development-lifecycle--流程框架}

安全保证的过程，重点是软件开发，但每个阶段都引入了安全和隐私的原则。

{{< figure src="https://i-blog.csdnimg.cn/blog_migrate/e233f5af457868b895045bd59cc8873c.png" >}}

{{< figure src="/ox-hugo/e233f5af457868b895045bd59cc8873c.png" width="800px" >}}


### <span class="section-num">2.2</span> 二、主要步骤 {#二-主要步骤}


#### <span class="section-num">2.2.1</span> 1.安全培训 Training {#1-dot-安全培训-training}


##### <span class="section-num">2.2.1.1</span> 1.1 Core Security Training 核心安全培训 {#1-dot-1-core-security-training-核心安全培训}

提高团队安全意识，对齐安全需求。

开发团队的所有成员都需要接受适当的安全培训，了解相关的安全知识，培训对象包括开发人员，测试人员、项目经理、产品经理等。项
目的中期可开展针对性的培训，例如代码中经常出现的问题，测试过程中多次出现的漏洞等等。


#### <span class="section-num">2.2.2</span> 2.需求分析 Requirements {#2-dot-需求分析-requirements}

针对系统业务要求实施风险评估工作，根据需求文档与项目经理确定安全需求，制定安全需求表，供后期检测时使用。


##### <span class="section-num">2.2.2.1</span> 2.1 建立安全需求分析 Establish Security Requirements {#2-dot-1-建立安全需求分析-establish-security-requirements}


##### <span class="section-num">2.2.2.2</span> 在项目确立之前，需要提前与项目经理或者产品 owner 进行沟通，确定安全的要求和需要做的事情。确认项目计划和里程碑，尽量避免因为安全问题而导致项目延期发布。 {#在项目确立之前-需要提前与项目经理或者产品-owner-进行沟通-确定安全的要求和需要做的事情-确认项目计划和里程碑-尽量避免因为安全问题而导致项目延期发布}


##### <span class="section-num">2.2.2.3</span> 2.2 创建质量标准及 Bug 栏 Create Quality Gates/ Bug Bars {#2-dot-2-创建质量标准及-bug-栏-create-quality-gates-bug-bars}

用于确定安全和隐私质量的最低可接受级别，在项目开始时定义这些标准可加强对安全问题相关风险的理解，并有助于团队在开发过程中
发现和修复安全 bug。项目团队必须协商确定每个开发阶段的质量门，随后将质量门交由给安全顾问审批，安全顾问可以根据需要添加特定
于项目的说明，以及更加严格的安全要求。另外，项目团队需阐明其对安全门的遵从性，以便完成最终安全评析（FSR）。

Bug 栏是应用于整个开发项目的质量门，用于定义安全漏洞的严重性阈值。例如，应用程序在发布时不得包含具有“关键”或“重要”评级的已
知漏洞。Bug 栏一经设定，便决不能放松。


##### <span class="section-num">2.2.2.4</span> 2.3 安全&amp;隐私风险评估（SRA&amp;PRA） {#2-dot-3-安全-and-隐私风险评估-sra-and-pra}

Security &amp; Privacy Risk Assessment

1.  项目的哪些部分在发布前需要威胁模型
2.  项目的哪些部分在发布前需要进行安全设计评析
3.  项目的哪些部分需要不属于项目团队且双方认可的小组进行渗透测试
4.  是否存在安全顾问认为有必要增加的测试或分析要求以缓解安全风险
5.  模糊测试的具体范围
6.  隐私对评级的影响


#### <span class="section-num">2.2.3</span> 3.系统设计 Design {#3-dot-系统设计-design}


##### <span class="section-num">2.2.3.1</span> 3.1 制定设计规范 Establish Design Requirements {#3-dot-1-制定设计规范-establish-design-requirements}

在设计阶段应该仔细考虑安全和隐私问题，在项目初期确定好安全需求，尽可能避免安全引起的需求变更。


##### <span class="section-num">2.2.3.2</span> 3.2 分析攻击面 Analyze Attack Surface {#3-dot-2-分析攻击面-analyze-attack-surface}

减小攻击面与威胁建模紧密相关，不过它解决安全问题的角度稍有不同。减小攻击面通过减小攻击者利用潜在弱点或漏洞的机会来降低风
险。方法包括：关闭或限制对系统服务的访问，应用“最小权限原则”，以及尽可能进行分层防御。


##### <span class="section-num">2.2.3.3</span> 3.3 威胁建模 Threat Modeling {#3-dot-3-威胁建模-threat-modeling}

为项目或产品面临的威胁建立模型，明确分析攻击可能来自哪里。微软提出的 STRIDE 模型以帮助建立危险模型，是非常好的做法，从 6 个维
度展开：

Spoofing（假冒）

Tampering（篡改）

Repudiation（否认）

Information Disclosure（信息泄漏）

Denial of Service（拒绝服务）

Elevation of Privilege（权限提升）。

其他主流威胁建模框架见文章《威胁建模主流框架》


#### <span class="section-num">2.2.4</span> 4.实现 Implementation {#4-dot-实现-implementation}


##### <span class="section-num">2.2.4.1</span> 4.1 使用优选工具 Use Approved Tools {#4-dot-1-使用优选工具-use-approved-tools}

开发团队使用的编辑器、链接器等相关工具，可能会涉及一些安全相关的环节，因此在使用工具的版本上，需要提前与安全团队进行沟通。


##### <span class="section-num">2.2.4.2</span> 4.2 消减危险函数 Deprecate Unsafe Functions {#4-dot-2-消减危险函数-deprecate-unsafe-functions}

许多常用函数可能存在安全隐患，应当禁用不安全的函数 API，使用安全团队推荐的函数。


##### <span class="section-num">2.2.4.3</span> 4.3 对代码进行静态安全检查（静态分析）Static Analysis {#4-dot-3-对代码进行静态安全检查-静态分析-static-analysis}

代码静态分析可以由相关工具辅助完成，其结果与人工分析相结合。


#### <span class="section-num">2.2.5</span> 5.验证 Verification {#5-dot-验证-verification}


##### <span class="section-num">2.2.5.1</span> 5.1 动态安全测试 Dynamix Analysis {#5-dot-1-动态安全测试-dynamix-analysis}

动态分析是静态分析的补充，用于测试环节验证程序的安全性。


##### <span class="section-num">2.2.5.2</span> 5.2 模糊测试 Fuzz Testing {#5-dot-2-模糊测试-fuzz-testing}

专门形态的动态分析，通过故意向应用程序引入不良格式或随机数据诱发程序故障。模糊测试策略的制定，以应用程序的预期用途，以及
应用程序的功能和设计规范为基础。安全顾问可能要求进行额外的模糊测试，或者扩大模糊测试的范围，增加持续的时间。


##### <span class="section-num">2.2.5.3</span> 5.3 供给面评审 Attack Surface Review {#5-dot-3-供给面评审-attack-surface-review}

项目经常会因为需求等因素导致最终的产出偏离原本设定的目标，因此在项目后期对威胁模型和攻击面进行评析是有必要的，能够及时发
现问题并修正。

所有新项目上线前要经过三步检查——代码审计、Web 应用扫描、人工渗透扫描。

代码审计：使用 Jekens 拉 Github 代码放入 Fortify 中扫描。开发过程中，开发人员每次更新代码都要进行扫描，并有权限查看 Fortify 相关
项目漏洞情况，进行整改（不允许有中高危以上漏洞）。开发有权对漏洞进行忽略处理，但需要承担相应后果。若不知道如何处理，可以
请安全组提供解决方案。

Web 应用扫描：Web 应用扫描不需要安全基础即可操作。安全人员比测试人员少，一般交予测试人员协助处理。扫描器可选择项很多，包括
开源或者商用。

人工渗透扫描：针对不同应用的发布情况，若是应用为新应用则需要对总体项目进行渗透测试。


#### <span class="section-num">2.2.6</span> 6.发布 Release {#6-dot-发布-release}


##### <span class="section-num">2.2.6.1</span> 6.1 制定安全应急响应计划 Incident Response Plan {#6-dot-1-制定安全应急响应计划-incident-response-plan}

受 SDL 要求约束的每个软件在发布时都必须包含事件响应计划。即使在发布时不包含任何已知漏洞的产品，也可能在日后面临新出现的威胁。
如果产品中包含第三方的代码，也需要留下第三方的联系方式并加入事件响应计划，以便在发生问题时能够找到对应的人。


##### <span class="section-num">2.2.6.2</span> 6.2 最终安全评审（FSR）Final Security Review {#6-dot-2-最终安全评审-fsr-final-security-review}

FSR 是在发布之前仔细检查对软件执行的所有安全活动。通过 FSR 将得出以下三种不同结果：

1.  通过 FSR。在 FSR 过程中确定所有安全和隐私问题都以得到修复或缓解。
2.  通过 FSR 但有异常。在 FSR 过程中确定的所有安全和隐私问题都以得到修复或缓解，并且/或者所有异常都已得到圆满解决，无法解决的
    问题将记录下来，在下次发布时更正。
3.  需上报问题的 FSR。如果团队未满足所有 SDL 要求，并且安全顾问和产品团队无法达成可接受的折中，则安全顾问不能批准项目，项目不
    能发布。团队必须在发布之前解决所有可解决的问题，或者上报高级管理层进行抉择。


##### <span class="section-num">2.2.6.3</span> 6.3 发布归档 Release Archive {#6-dot-3-发布归档-release-archive}

在通过 FSR 或者虽有问题但达成一致后，可以完成产品的发布。但发布的同时仍需对各种问题和文档进行存档，为紧急响应和产品升级提供
帮助。


#### <span class="section-num">2.2.7</span> 7.响应 Response {#7-dot-响应-response}

Execute Incident Response Plan 执行安全应急响应计划。


### <span class="section-num">2.3</span> 三、SDL 实战经验准则 {#三-sdl-实战经验准则}


#### <span class="section-num">2.3.1</span> 1.与项目经理进行充分沟通，排除足够的时间 {#1-dot-与项目经理进行充分沟通-排除足够的时间}

    项目的安全评估，在开发的不同环节有着不同的安全要求，而这些安全要求都需要占用开发团队的时间，因此在立项阶段与项目经理
进行充分沟通十分重要。

明确在什么阶段安全工程师需要介入，需要多长时间完成安全工作，同时预留多少时间给开发团队用以开发安全功能或者修复安全漏洞。

预留出必要时间对项目的时间管理也具有积极意义，否则很容易出现项目快发布，安全团队突然说还没有实施安全检查的情况。这种情况
只能导致两种结果：项目因为安全检查而延期发布，开发团队、测试团队全员一起重新做安全检查；项目顶着风险发布，专门建立小项目
专门修复安全问题。

两种结果都十分糟糕，为避免发生此类情况，在立项初期就应该与项目经理进行充分沟通，留出足够多的时间给安全检查。


#### <span class="section-num">2.3.2</span> 2.规范公司的立项流程，确保所有项目都能通知到安全团队，避免遗漏 {#2-dot-规范公司的立项流程-确保所有项目都能通知到安全团队-避免遗漏}

安全事件产生的原因并不复杂，往往发生在大家忽略的地方。在实施 SDL 的过程中，技术方案的好坏不是最关键的，最关键的是要覆盖到全
部项目，防止边边角角的小项目没有被覆盖到导致安全事件的发生。

公司规模较小时，员工沟通成本低，很容易保证所有项目都能及时通知到安全团队。但公司大到一定规模时，出现多个部门和多个项目组，
沟通成本大大增加，这种情况下就很有必要从公司层面建立完善的“立项制度”。

SDL 依托于软件工程，立项也属于软件工程的一部分。如果能集中管理立项过程，SDL 就有可能在这一阶段覆盖到公司所有项目。相对于测
试阶段和发布阶段来说，在立项阶段就有安全团队介入，留给开发团队的反应事件也更加富足。


#### <span class="section-num">2.3.3</span> 3.树立安全部门的权威，项目必须由安全部门审核完成后才能发布 {#3-dot-树立安全部门的权威-项目必须由安全部门审核完成后才能发布}

实施 SDL 的过程中，除了教育项目组成员（项目经理、产品经理、开发人员、测试人员等）实施安全的好处外，安全部门还需要树立一定的
权威。

必须通过规范和制度，明确要求所有项目必须在安全审核完成后才能发布。如果没有这样的权威，对于项目组来说，安全就变成了一项可
有可无的东西。如果产品急着发布，很可能因此砍掉或裁减部分安全需求，也可能延期修补漏洞，从而导致风险升高。

权威的树立在公司里需要从上往下推动，由技术总负责人或者产品总负责人确认安全部门实施。在具体实施时可以依据公司的不同情况在
相应的流程中明确。负责产品的质量保障部门，负责产品发布的运维部门，都可以称为制度的执行者。

“项目必须由安全部门审核完成之后才能发布”这句话并非绝对，背后含义实为树立安全部门的权威。因此在实际实施 SDL 的过程中，安全也
可能对业务妥协。因此在业务时间压力非常大，问题不是很严重的情况下，可以考虑事后再进行修补，或使用临时方案应对紧急情况。安
全最终是为业务服务的。


#### <span class="section-num">2.3.4</span> 4.将技术方案写入开发、测试的工作手册中 {#4-dot-将技术方案写入开发-测试的工作手册中}

对于开发、测试团队，对工作最有效的约束方式即工作手册。对于开发来说手册即为开发规范。开发规范涉及的方面比较广，如函数名的
大小写方式、注释的写法等。（腾讯开源开发安全指南链接如下：腾讯代码安全指南开源，涉及 CC++、Go 等六门编程语言 - FreeBuf 网络
安全行业门户）

但开发团队的规范内容鲜有涉及到安全规范，因此与其事后通过代码审核的方式告知开发者代码中存在漏洞，需要修补，不如直接将安全
技术方案写入开发者的代码规范当中。比如规定好哪些函数禁用，只能使用哪些函数；或封装好安全功能，在代码规范中注明什么情况下
使用什么样的安全 API。

对于程序员来说，记住代码规范中的要求远比记住复杂的安全原理要容易得多。一般程序员只需要记住如何使用安全功能即可，无需深究
原理。

对于测试人员的要求是相似的。在测试的工作手册中加入安全测试的方法，清楚列出每一个测试用例，每一步实现什么功能，这样一些基
础的安全测试就可以交由测试人员完成，最后生成一份安全测试报告即可。


#### <span class="section-num">2.3.5</span> 5.给工程师培训安全方案 {#5-dot-给工程师培训安全方案}

微软 SDL 框架中，第一项就是培训。培训的作用不可小视，它是技术方案与执行者之间的调和剂。在准则四中提到，要将安全技术方案最大
程度地写入代码规范等工作手册中，但同时要让开发者有机会了解到安全方案地背景，这也是很有意义的，可以通过培训达到这个目的。

培训最重要的目的是在项目开发之前，能够让开发者之傲如何写出安全的代码，从而节约开发成本。如果开发者未经培训，可能在代码审
核阶段会被找出非常多的安全 bug，修复每一个安全 bug 都将消耗额外的开发时间；同时开发者若不理解这些开发问题，由安全工程师对每
一个问题进行解释和说明也是一份额外的时间支出。

因此在培训阶段贯彻代码规范中的安全需求，可以极大地节约开发时间，对整个项目组都有着积极的意义。


#### <span class="section-num">2.3.6</span> 6.记录所有的安全 bug，激励程序员编写安全的代码 {#6-dot-记录所有的安全-bug-激励程序员编写安全的代码}

为了更好地推动项目组写出安全的代码，可以尝试给每个开发团队设立绩效。被发现漏洞最少的团队可以得到奖励，并将结果公布出来。
如此，项目组之间将产生竞争氛围，开发者更努力于遵守安全规范，写出安全的代码，还能帮助不断提高开发者的代码质量，形成良性循
环。


### <span class="section-num">2.4</span> 四、与 SAMM 对比 {#四-与-samm-对比}

相对于微软的 SDL，OWASP 推出了 SAMM（Software Assurance Maturity Model），帮助开发者在软件工程的过程中实施安全。

SAMM 与 SDL 的主要区别在于，SDL 适用于软件开发商，他们以贩售软件为主要业务；而 SAMM 更适用于自主开发软件的使用者，如银行或在线
服务提供商。软件开发商的软件工程往往较为成熟，有着严格的质量控制；而自主开发软件的企业组织，则更强调高效，因此在软件工程
的做法上也存在差异。

{{< figure src="https://i-blog.csdnimg.cn/blog_migrate/431b7151577d7c784be5a929cd960aba.png" >}}

{{< figure src="/ox-hugo/431b7151577d7c784be5a929cd960aba.png" width="800px" >}}

OWASP SAMM | OWASP Foundation


### <span class="section-num">2.5</span> 五、常用 SDL 实施方法和工具 {#五-常用-sdl-实施方法和工具}


#### <span class="section-num">2.5.1</span> 1、需求分析与设计阶段 {#1-需求分析与设计阶段}

项目初始阶段，将论证项目的目标、可行性、实现方向等。在需求阶段，安全工程师需要关心产品主要功能上的安全强度和安全体验是否
足够，主要需要思考安全功能。在此阶段可以对项目经理、产品经理或者架构师进行访谈，以了解产品背景和技术架构，并给出相应的建
议，以下是安全专家 Lenny Zeltser 给出的一份 cheklist


##### <span class="section-num">2.5.1.1</span> #1:BUSINESS REQUIREMENTS {#1-business-requirements}


##### <span class="section-num">2.5.1.2</span> #1:业务需求 {#1-业务需求}

Business Model

商业模型

1.What is the application’s primary business purpose?

这款应用的主要业务目的是什么？

2.How will the application make money?

这款应用如何盈利？

3.What are the planned business mlestones for developing or imporving the application?

开发或者改进这款应用的业务计划是什么样的？

4.How is the application marketed?

该应用是如何进行营销的？

5.What key benefits does the application offer users?

这款应用给用户提供的核心功能是什么？

6.What business continuity provisions have been defined for the application?

已经为该应用制定了哪些业务连续性规定？

7.What geographic areas does the application service?

这款应用提供服务的主要地区？

Data Essentials

必要数据

1.What data does the application receive, produce and process?

这款应用接收、产生、处理了什么数据？

2.How can the data be classified into categories according to sensitivity?

哪些属于敏感数据？

3.How might an attacker benefit from capturing or modifying the data?

捕获并修改数据将如何有利于攻击者？

4.What data backup and retention requirements have been defined for application?

已经为应用制定了哪些数据备份和保留要求？

End - Users

终端用户

1.Who are the application’s end - users?

哪些人是应用的最终用户？

2.How do the end - user interact with the application?

最终用户如何与应用交互？

3.What security expectations do the end - users have?

最终用户有哪些安全期望？

Partners

搭档

1.Which third - parties supply data to the application?

哪些第三方组织为应用提供数持？

2.Which third - parties receive data from the applications?

哪些第三方组织从应用中收集数据？

3.Which third - parites process the application’s data?

哪些第三方组织处理应用数据？

4.What mechanisms are used to share data with third - parties besides the application itself?

除了应用本身，还有那些机制被用来和第三方共享数据？

5.What security requirements do the partners impose?

合作伙伴实施了哪些安全要求？

Administrators

管理层

1.Who has administrative capabilities in the application?

谁对应用有着管理员权限？

2.What administrative capabilities does the application offer?

应用提供哪些管理员权限功能？

Regulations

规章制度

1.In what industries does the application operate?

该应用在哪些行业中使用？

2.What security - related regulations apply?

适用哪些与安全相关的法规？

3.What auditing and compliance regulations apply?

适用于哪些审计和法规？


##### <span class="section-num">2.5.1.3</span> #2：INRASTRUCTURE REQUIREMENTS {#2-inrastructure-requirements}


##### <span class="section-num">2.5.1.4</span> 内部结构要求 {#内部结构要求}

Network

网络

1.What details regarding routing, switching, firewalling, and load- balancing have been defined?

定义了哪些有关路由、交换、防火墙和负载平衡的细节？

2.What network design supports the application?

进行了怎样的网络设计来支持该应用程序？

3.What core network devices support the application?

哪些核心网络设备用来支持应用程序？

4.What network performance requirements exist?

存在哪些网络性能要求？

5.What private and public network links support the application?

有哪些私有和公共网络链接支持应用程序？

Systems

系统

1.What operating systems support the application?

应用基于哪款操作系统？

2.What hardware reqirements have been defined?

定义了哪些硬件需求？

3.What details regarding required OS components and lock - down needs have been defined?

定义了哪些所需操作系统组件和锁定需求的细节？

Infrastructure Monitoring

基础设施监控

1.What network and system performance monitoring requirements have been defined？

定义了哪些网络和系统性能监控要求？

2.What mechanisms exist to detect malicious code or compromised applicaton components?

有哪些机制用于检测恶意代码或损坏的应用程序组件？

3.What network and system security monitoring requirements have been defined?

定义了哪些网络和系统的安全监控要求？

Virtualization and Externalization

虚拟化和外部化

1.What aspects of the application lend themselves to virtualization?

该应用哪些方面适合虚拟化？

2.What virtualization requirements have been defined for the applications?

该程序已经定义了哪些虚拟化需求？

3.What aspects of the product may or may not be hosted via the cloud coompting model?

该产品哪些方面可以或者不可以被云聚合模型托管？


##### <span class="section-num">2.5.1.5</span> #3：APPLICATION REQUIREMENTS {#3-application-requirements}


##### <span class="section-num">2.5.1.6</span> 应用程序要求 {#应用程序要求}

Environment

环境

1.What frameworks and programming languages have been used to create the application?

应用使用了什么样的框架和编程语言？

2.What process, code, or infrastructure dependencies have been defined for the application?

应用定义了哪些流程、代码和基础设施依赖关系？

3.What databases and application servers support the application?

应用由哪些数据库和应用程序服务器支持？

Data Processing

数据处理

1.What data entry paths does the application support?

该应用支持哪些数据输入方式？

2.What data output paths does the ap[lication support?

该应用支持哪些数据输出方式？

3.How does data flow across the application’s internal components?

数据如何进行跨程序内部组件流动？

4.What data input validation requirements have been defined?

定义了哪些数据输入验证要求？

5.What data does the application store and how?

应用存储了哪些数据？如何存储的？

6.What data is or may not to be encrypted and what key management requirements have been defined?

哪些数据被加密或者没有被加密？定义了哪些密钥管理要求？

7.What capabilities exist to detect the leakage of sensitive data?

有哪些方法用于检测敏感数据泄露？

8.What encryption requirements have been defined for data in transit over WAN and LAN links?

为通过 WAN 和 LAN 传输的数据制定了哪些加密要求？

Access

访问

1.What user identification and authentication requirements have been defined?

使用了哪些用户身份识别和身份验证要求？

2.What session management requirements have been defined?

定义了哪些会话管理要求？

3.What access requirements have been defined for URI and Service calls?

为 URI 和服务呼叫定义了哪些访问要求？

4.What user authorization requirements have been defined?

定义了哪些用户授权要求？

5.How are user identities maintained throughout transaction calls?

怎样在整个事务调用过程中维护用户身份？

6.What user access restrictions have been defined?

定义了哪些访问权限？

7.What user privilege levels does the application support?

应用支持哪些用户特权级别？

Application Monitoring

应用监控

1.What application performance monitoring requirements have been defined？

定义了哪些应用程序性能监控要求？

2.What application security monitoring requirements have been defined?

定义了哪些应用程序安全监控要求？

3.What application error handling and logging requirements have been defined?

定义了哪些应用程序错误处理和日志记录要求？

4.How are audit and debug logs accessed, stored, and secured?

如何访问、存储和安全地保护审计和调试日志？

5.What application auditing requirements have been defined?

定义了哪些应用程序审核要求？

Application Design

应用设计

1.How many logical tiers group the application’s components?

多少个逻辑层组成了应用组件？

2.How is intermediate or in process data stored in the application components’ memory and in cache?

中间数据或进程中的数据如何存储在应用程序组件的内存和缓存中？

3.What application design review practices have been defined and executed?

定义并执行了哪些应用程序设计审查实践？

4.What staging, testing, and Quality Assurance requirements have been defined?

定义了哪些分段、测试和质量保证要求？


##### <span class="section-num">2.5.1.7</span> #4:SECURITY PROGRAM REQUIREMENTS {#4-security-program-requirements}


##### <span class="section-num">2.5.1.8</span> 安全程序要求 {#安全程序要求}

Operations

操作

1.What access to system and network administrators have to the application’s sensitive data?

系统和网络管理员对应用程序的敏感数据有什么访问权限？

2.What security incident requirements have been defined?

定义了哪些安全事件要求？

3.What physical controls restrict access to the applications’s components and data?

哪些物理控制限制了对应用程序的组件和数据的访问？

4.What is the process for granting access to the environment hosting the application?

授予对托管该应用程序的环境的访问权的过程是怎样的？

5.What is the process for identifying and addressing vulnerabilities in network and system components?

识别和解决网络和系统组件中的漏洞的过程是怎样的？

6.How do administrators access production infrastructure to manage it?

管理员如何访问及管理生产基础设施？

7.What is the process for identifying and addressing vulnerabilities in the applications?

识别和解决应用程序中的漏洞的过程是怎样的？

Change Management

管理变动

1.What mechanisms exist to detect violations of change management practice?

存在哪些机制来检测违反变更管理实践的行为？

2.How are changes to the infrasturcture controlled?

如何控制基础设施结构的变化？

3.How are changes to the code controlled?

如何控制对代码的更改？

4.How is code deployed to production?

如何将代码部署到生产过程中？

Software Development

软件开发

1.How do developers assist with troubleshooting and debugging the application?

开发人员如何协助进行故障排除和调试应用程序？

2.What requirements have been defined for controlling access to the applications source code?

为控制对应用程序源代码的访问，定义了哪些要求？

3.What data is available to developers for testing?

开发人员可以用哪些数据进行测试？

4.What secure coding processes have been established?

建立了哪些安全的编码流程？

Corporate

合作

1.Which personnel oversees security processes and requirements related to te application?

哪些人员负责监督与技术申请相关的安全流程和需求？

2.What employee initiation and termination procedures have been defined?

定义了哪些员工启动和终止程序？

3.What controls exist to protect a compromised in the corporate environment from affecting production？

有哪些控制措施可以保护企业环境中不影响生产？

4.What security governance requirements have been defined?

定义了哪些安全治理要求？

5.What security training do developers and administrators undergo?

开发人员和管理员接受过什么安全培训？

6.What application requirements impose the need to enforce the principle of separation of duties?

什么要求执行了职责分离原则？

7.What corporate security program requirements have been defined?

定义了哪些企业安全计划要求？


#### <span class="section-num">2.5.2</span> 2.开发阶段 {#2-dot-开发阶段}

依据“安全是为业务服务”这一指导思想，在需求层面，安全改变业务的地方较少，应当力求代码实现上的安全，做到“安全的功能”。


##### <span class="section-num">2.5.2.1</span> (1)提供安全的函数 {#1--提供安全的函数}

ESAPI 是 OWASP 提供的一套 API 级别的 web 应用解决方案。简单的说，ESAPI 就是为了编写出更加安全的代码而设计出来的一些 API，方便使用
者调用，从而方便的编写安全的代码。其中 Java 版本最为完善。

其官方网站为：<https://www.owasp.org/www-project-enterprise-security-api/%EF%BC%8C%E5%85%B6%E6%9C%89%E5%BE%88%E5%A4%9A%E9%92%88%E5%AF%B9%E4%B8%8D%E5%90%8C%E8%AF%AD%E8%A8%80%E7%9A%84%E7%89%88%E6%9C%AC%EF%BC%8C%E5%85%B6J2ee%E7%9A%84%E7%89%88%E6%9C%AC%E9%9C%80%E8%A6%81>
jre1.5 及以上支持。

WEB 安全编程技术规范(V1.0) - 百度文库（Java 安全编程 ESAPI）

将安全功能放到开发框架中实现，会大大降低程序员的开发工作量。还可以使用的一个实践就是将安全技术写进开发规范中，让开发者牢
记开发规范。在代码审核阶段，通过白盒扫描的方式检查变量输出是否使用了安全的函数，没有使用安全函数即可认为不符合安全规范。

将安全方案写入开发规范中，就真正让安全方案落了地，不仅是为方便来发着写出安全的代码，同时也为代码安全审计带来了方便。


##### <span class="section-num">2.5.2.2</span> (2)代码安全审计工具 {#2--代码安全审计工具}

常见代码审计攻击在面对复杂项目时会束手无策。原因如下：函数调用是一个复杂过程，常有一个函数调用另一个文件中函数的情况。当
代码审计工具找到敏感函数 eval()时，回溯函数的调用路径遇到困难。其次，如果程序使用复杂框架，代码审计工具也缺乏对框架的支持，
导致误报和漏报。

另一种思路就是找到所有可能的用户输入入口，跟踪变量的传递情况，看变量最后是否会走到危险函数。

目前代码审计的结果仍然需要人工处理，下表列出常见代码审计工具。

Name  Type  Description
BOON  academic  A model checker that targets buffer-overflow vulnerabilities in C code.
Bugscan  open source  Checks for potentially dangerous function calls in binary executable code.

  /commercial
CodeAssure  commercial  General-purpose security scanners for many programming languages.
CodeSonar  commercial  Checks for vulnerabilities and other defects in C and C++.
CodeSpy  Open source  Security scanner for Java.
Fortify Tools  commercial  General-purpose security scanners for many C,C++, and Java
PrexisEngine  commercial  Security scanner for C/C++ and Java/JSP.

代码自动化审计较为困难，但甲方公司可以根据开发规范来丁真代码审计攻击。核心思想：并非直接检查代码是否安全，而是检查开发者
是否遵守了开发规范。将复杂的“代码自动化审计”问题转化为“代码是否符合开发规范“，而开发规范在编写时就可以写成易于审计的规范
最终，如果开发规范中的安全方案没有问题，开发者严格遵守开发规范，产出的代码基本上也是安全的。


##### <span class="section-num">2.5.2.3</span> (3)测试阶段 {#3--测试阶段}

    产品发布前最后一个阶段，此阶段要对产品进行充分的安全测试，验证需求分析、设计阶段的安全功能是否符合预期目标，并验证在
开发阶段发现的所有安全问题是否得到解决。

安全测试独立于代码审计存在，相对于代码审计而言至少有两个好处：1）有一些代码逻辑较为复杂，可以通过代码审计发现所有问题；2）
有一些逻辑漏洞通过安全测试可以更快得到结果。

安全测试分为自动化和手工两种。自动化以覆盖性测试为主，可通过 web 应用扫描器对项目或产品进行漏洞扫描。

而对“CSRF”、“越权访问”、“文件上传”等漏洞，难以达到自动化检测的效果，因为涉及到系统逻辑或者业务逻辑，有时需要人机交互参与
页面流程，故会一来到手动测试。


### <span class="section-num">2.6</span> 六、敏捷 SDL {#六-敏捷-sdl}

微软 SDL 过程仍然较为厚重，适用于采用瀑布法进行开发的软件开发团队，对于使用敏捷开发的团队难以适应。

敏捷开发采用“小步快跑”方式，不断完善产品，没有非常规范的流程，文档尽可能简单，有利于产品的快速发布，但不利于安全，需求无
法在一开始非常明确，导致安全设计发生变化，故微软设计了敏捷 SDL。

敏捷 SDL 即以变化的观点实施安全工作。需求和功能可能一直变化，代码也可能在变化，于是要求了实施 SDL 时需要在每个阶段更新威胁模
型和隐私政策，在必要的环节迭代模糊测试、代码安全分析等工作。


### <span class="section-num">2.7</span> 七、软件安全开发流程新增的安全活动 {#七-软件安全开发流程新增的安全活动}

传统软件开发流程  软件安全开发流程新增项
客户需求收集  客户安全需求收集
需求分析与澄清  分析客户安全需求，制定安全标准和要求，建立安全需求管理，安全与隐私风险评估
软件设计  在特性设计中结合安全设计规范进行安全设计，对模块进行威胁建模和攻击面分析
软件编码  1. 安全编码：新代码使用安全函数，老代码替换危险函数，对代码进行安全检视

1.  使用第三方开源工具或者库

版本构建  1. 满足安全编译选项的要求

1.  选择安全的构建工具

软件测试  1. 对安全设计进行分析和评审，输出安全测试用例

1.  对软件暴露的接口进行 Fuzz 测试
2.  对软件暴露的攻击面进行攻防渗透测试
3.  对软件进行安全扫描和漏洞挖掘

客户质量验收  1. 提供验收的安全交付件，如安全设计说明书、安检测报告

1.  提供测试环境给用户，让用户进行安全评估

发布和运维  1. 制定安全应急响应计划，并在实际运维中落地

1.  日常运维中做好安全巡检，遇到黑客攻击，即使告警，并快速修复安全漏洞


### <span class="section-num">2.8</span> 八、附件《安全选项表》 {#八-附件-安全选项表}

其中的内容是等级保护和网站备案中必须存在的一些安全功能项，属于法律合规内容，不是产品必须符合其中的内容，而是尽量符合，自
行选择保留项。

开发和测试根据产品填写好的《安全选项表》内容进行测试。部分安全专业的测试工作由安全人员进行测试。检测完成并允许上线后，三
份测试报告和安全选项表格要进行归档。

Reference：

<https://blog.csdn.net/a167220278/article/details/80454756?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522164428096416780269873953%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fall.%2522%257D&request_id=164428096416780269873953&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~first_rank_ecpm_v1~rank_v31_ecpm-1-80454756.pc_search_result_cache&utm_term=%E5%AE%89%E5%85%A8%E5%BC%80%E5%8F%91%E7%94%9F%E5%91%BD%E5%91%A8%E6%9C%9F&spm=1018.2226.3001.4187>

