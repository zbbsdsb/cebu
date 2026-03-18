# Phase 9: 荒谬度系统完整文档

## 概述

Phase 9 实现了完整的荒谬度系统，将模糊逻辑、随机微分方程、拓扑变形与戏剧叙事深度集成。Phase 9 包含三个子模块：

- **Phase 9a**: 荒谬度核心系统
- **Phase 9b**: 拓扑变形系统
- **Phase 9c**: 叙事上下文系统

## 系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                     Phase 9: 荒谬度系统                      │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  Phase 9c: Narrative Context                                  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ Story Beats  │→ │ Narrative    │→ │ Drama        │      │
│  │ (17 types)   │  │ Analysis     │  │ Templates    │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│        │                 │                 │                │
│        ▼                 ▼                 ▼                │
│  ┌──────────────────────────────────────────────────┐        │
│  │  NarrativeContext (S, L, E, T, D)                │        │
│  │  - get_driving_force()  → κ·S + η·L             │        │
│  │  - get_volatility()    → σ₀ · (1 + E) · (1 + T) │        │
│  └──────────────────────────────────────────────────┘        │
│                           │                                   │
│                           ▼                                   │
│  Phase 9a: Absurdity System                                   │
│  ┌──────────────────────────────────────────────────┐        │
│  │  FuzzyInterval                                   │        │
│  │  - Value + Uncertainty                           │        │
│  │  - Interval arithmetic                           │        │
│  └──────────────────────────────────────────────────┘        │
│                           │                                   │
│                           ▼                                   │
│  ┌──────────────────────────────────────────────────┐        │
│  │  SDEEvolution ( stochastic differential eq)       │        │
│  │  dA = [f(A, t) + κ·S + η·L]dt + σ·dW              │        │
│  └──────────────────────────────────────────────────┘        │
│                           │                                   │
│                           ▼                                   │
│  ┌──────────────────────────────────────────────────┐        │
│  │  AbsurdityField                                   │        │
│  │  - Spatial mapping of absurdity                  │        │
│  │  - Multi-source fusion (6 methods)              │        │
│  └──────────────────────────────────────────────────┘        │
│                           │                                   │
│                           ▼                                   │
│  Phase 9b: Topology Morph                                     │
│  ┌──────────────────────────────────────────────────┐        │
│  │  TopologyMorphSystem                              │        │
│  │  - 10 morph operations                           │        │
│  │  - 6 trigger conditions                          │        │
│  │  - 6 preset rules                                │        │
│  │  - Delay queue system                            │        │
│  └──────────────────────────────────────────────────┘        │
│                           │                                   │
│                           ▼                                   │
│  ┌──────────────────────────────────────────────────┐        │
│  │  Simplicial Complex                               │        │
│  │  - Dynamic topology changes                      │        │
│  │  - Refinement based on absurdity                 │        │
│  └──────────────────────────────────────────────────┘        │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

## Phase 9a: 荒谬度核心系统

### 核心组件

#### 1. FuzzyInterval - 模糊区间数

表示不确定的数值，包含中心值和不确定性范围。

```cpp
FuzzyInterval a(0.5, 0.1);  // value = 0.5, uncertainty = 0.1
// Interval: [0.4, 0.6]

// 运算
FuzzyInterval b = a + FuzzyInterval(0.3, 0.05);
FuzzyInterval c = a * 2.0;
FuzzyInterval d = a.sqrt();
```

**特性**:
- 区间算术运算（+、-、*、/）
- 标量运算（*、/）
- 数学函数（sqrt、log、exp、pow）
- 比较运算（contains、overlaps、is_subset）
- 不确定性度量（width、radius、entropy）

#### 2. SDEEvolution - 随机微分方程演化

实现随机微分方程驱动的荒谬度演化：

```
dA(t) = [f(A, t) + κ·S + η·L]·dt + σ·dW(t)
```

其中：
- `A(t)`: 荒谬度
- `f(A, t)`: 漂移项
- `κ·S + η·L`: 耦合项（叙事驱动）
- `σ·dW(t)`: 扩散项（随机噪声）

```cpp
SDEEvolution evolution;
SDEEvolutionParams params;
params.volatility = 0.1;
params.coupling_strength = 0.5;
params.time_step = 0.01;

FuzzyInterval current(0.5, 0.1);
FuzzyInterval next = evolution.step(current, params);
```

**特性**:
- Euler-Maruyama 数值积分
- 可配置漂移、耦合、扩散参数
- 批量演化（N 步）
- 演化轨迹跟踪

#### 3. MultiSourceFusion - 多源融合

融合多个荒谬度来源：

```cpp
MultiSourceFusion fusion;

// 来源 1: 叙事驱动
std::vector<FuzzyInterval> source1 = {a1, a2, a3};

// 来源 2: 用户交互
std::vector<FuzzyInterval> source2 = {b1, b2, b3};

// 来源 3: 环境因素
std::vector<FuzzyInterval> source3 = {c1, c2, c3};

// 加权平均融合
auto fused = fusion.weighted_average({source1, source2, source3}, {0.5, 0.3, 0.2});
```

**融合方法** (6 种):
1. **Weighted Average**: 加权平均
2. **Weighted Median**: 加权中位数
3. **Probabilistic Mixture**: 概率混合
4. **Dempster-Shafer**: 证据理论融合
5. **Interval Intersection**: 区间交集
6. **Custom**: 自定义融合函数

#### 4. AbsurdityField - 荒谬度场

空间分布的荒谬度：

```cpp
AbsurdityField field(3, 3, 3);  // 3D 网格

// 设置值
field.set(0, 0, 0, FuzzyInterval(0.5, 0.1));

// 获取值
auto value = field.get(1, 1, 1);

// 插值查询
auto interpolated = field.interpolate(0.5, 0.5, 0.5);

// 梯度计算
auto gradient = field.gradient(1, 1, 1);
```

**特性**:
- 2D/3D 网格存储
- 线性/双线性/三线性插值
- 梯度计算
- 多源融合
- 持久化（JSON/Binary）

#### 5. AbsurdityLabel - 荒谬度标签

将荒谬度关联到单纯形：

```cpp
SimplicialComplexLabeled<FuzzyInterval> complex;

auto v0 = complex.add_simplex({}, FuzzyInterval(0.5, 0.1));
auto v1 = complex.add_simplex({}, FuzzyInterval(0.6, 0.15));

// 查询高荒谬度单纯形
auto high = complex.get_high_labeled_simplices(FuzzyInterval(0.7, 0.1));

// 范围查询
auto range = complex.get_labeled_simplices_in_range(
    FuzzyInterval(0.3, 0.1),
    FuzzyInterval(0.7, 0.1)
);
```

---

## Phase 9b: 拓扑变形系统

### 核心组件

#### 1. TopologyMorphSystem - 变形系统

执行拓扑变形操作：

```cpp
TopologyMorphSystem morph_system;

// 应用变形规则
morph_system.apply(ChaosMorphRules(), complex, 0.8);

// 触发所有变形
morph_system.trigger_all_morphs(complex, 1.0);

// 执行延迟队列
morph_system.execute_delayed(complex);
```

#### 2. MorphOperation - 变形操作

10 种变形操作：

| 操作 | 描述 | 效果 |
|------|------|------|
| Split Edge | 分割边 | 插入中点 |
| Collapse Edge | 折叠边 | 合并端点 |
| Flip Edge | 翻转边 | 改变连接 |
| Subdivide Triangle | 细分三角形 | 分成 4 个 |
| Smooth | 平滑 | 移动顶点 |
| Noise | 噪声 | 随机位移 |
| Warp | 扭曲 | 非线性变形 |
| Dilate | 膨胀 | 扩大体积 |
| Erode | 腐蚀 | 缩小体积 |
| Chamfer | 倒角 | 创建圆角 |

#### 3. MorphTrigger - 触发条件

6 种触发条件：

| 条件 | 描述 | 阈值 |
|------|------|------|
| Absurdity Threshold | 荒谬度阈值 | A > 0.7 |
| Gradient Threshold | 梯度阈值 | |∇A| > 0.5 |
| Time Delay | 时间延迟 | t > τ |
| Event Trigger | 事件触发 | 指定事件 |
| User Trigger | 用户触发 | 手动 |
| Random Trigger | 随机触发 | 概率 p |

#### 4. MorphRules - 变形规则

6 种预设规则：

| 规则 | 触发条件 | 变形操作 | 描述 |
|------|---------|---------|------|
| Gentle Rules | A < 0.3 | Smooth | 温和变形 |
| Moderate Rules | 0.3 ≤ A < 0.6 | Split + Noise | 中等变形 |
| Chaos Rules | A ≥ 0.6 | All morphs | 混沌变形 |
| Gradient Rules | |∇A| > 0.5 | Warp + Chamfer | 梯度变形 |
| Temporal Rules | 时间延迟 | Delayed queue | 时间变形 |
| Interactive Rules | 用户触发 | Selected ops | 交互变形 |

#### 5. MorphStatistics - 统计跟踪

跟踪变形统计：

```cpp
MorphStatistics stats = morph_system.get_statistics();

std::cout << "Total morphs: " << stats.total_morphs << "\n";
std::cout << "By operation:\n";
for (const auto& [op, count] : stats.morphs_by_operation) {
    std::cout << "  " << static_cast<int>(op) << ": " << count << "\n";
}
```

---

## Phase 9c: 叙事上下文系统

### 核心组件

#### 1. NarrativeContext - 叙事上下文

包含 5 个叙事维度：

```cpp
NarrativeContext ctx(
    0.8,  // Surprisal (S) - 意外性
    0.7,  // Logic Deviation (L) - 逻辑偏离度
    0.9,  // Emotional Intensity (E) - 情绪强度
    0.9,  // Dramatic Tension (T) - 戏剧张力
    0.0   // Time Distance (D) - 时间距离
);

// 获取驱动力（用于 SDE）
double driving_force = ctx.get_driving_force();  // κ·S + η·L

// 获取波动率（用于 SDE）
double volatility = ctx.get_volatility();        // σ₀ · (1 + E) · (1 + T)
```

#### 2. DramaticEventType - 戏剧事件类型

17 种事件类型：

**经典事件 (9 种)**:
- Exposition (铺垫)
- Inciting Incident (激励事件)
- Rising Action (上升动作)
- Plot Point 1 (第一转折)
- Midpoint (中点)
- Plot Point 2 (第二转折)
- Climax (高潮)
- Falling Action (下降动作)
- Resolution (结局)

**特殊事件 (8 种)**:
- Twist (剧情转折)
- Revelation (揭秘)
- Conflict (冲突)
- Transition (过渡)
- Pause (暂停)
- Flashback (闪回)
- Flashforward (闪前)
- Montage (蒙太奇)
- Time Skip (时间跳转)
- Custom (自定义)

#### 3. StoryBeat - 故事节拍

```cpp
StoryBeat beat(
    DramaticEventType::CLIMAX,
    NarrativePresets::climax(1.0)
);

std::cout << beat.to_string() << "\n";
// 输出: Climax: S=1.0, L=0.8, E=1.0, T=1.0, D=0.0
```

#### 4. NarrativeTemplate - 叙事模板

4 种叙事结构：

| 结构 | 节拍数 | 描述 |
|------|--------|------|
| Three-Act Structure | 9 | 经典三幕式结构 |
| Hero's Journey | 12 | 英雄之旅 |
| Save the Cat | 15 | 救猫咪 |
| Custom | 可变 | 自定义结构 |

```cpp
// 获取三幕式结构
auto beats = NarrativeTemplate::three_act_structure();

// 获取英雄之旅
auto hero_journey = NarrativeTemplate::heros_journey();
```

#### 5. NarrativeAnalyzer - 叙事分析器

```cpp
NarrativeAnalyzer analyzer(beats);

// 分析特定位置
auto ctx = analyzer.analyze_at(5);

// 检测高潮
if (analyzer.is_approaching_climax(5)) {
    std::cout << "Approaching climax!\n";
}

// 预测下一个事件
auto next = analyzer.predict_next_type(5);
```

---

## 完整工作流程

### 示例 1: 叙事驱动的荒谬度演化

```cpp
#include "cebu/narrative_context.h"
#include "cebu/absurdity_evolution.h"

using namespace cebu;

// 1. 创建故事结构
auto beats = NarrativeTemplate::three_act_structure();

// 2. 初始化荒谬度
FuzzyInterval absurdity(0.5, 0.1);
SDEEvolution evolution;

// 3. 遍历故事节拍
for (const auto& beat : beats) {
    std::cout << beat.to_string() << "\n";
    
    // 4. 配置演化参数
    SDEEvolutionParams params;
    params.volatility = beat.context.get_volatility();
    params.coupling_strength = beat.context.get_driving_force();
    params.diffusion_strength = 0.1;
    params.time_step = 0.01;
    
    // 5. 演化荒谬度
    absurdity = evolution.step(absurdity, params);
    
    std::cout << "  → Absurdity: " << absurdity.to_string() << "\n\n";
}
```

### 示例 2: 荒谬度驱动的拓扑变形

```cpp
#include "cebu/narrative_context.h"
#include "cebu/absurdity_evolution.h"
#include "cebu/topology_morph.h"

using namespace cebu;

// 1. 创建拓扑变形系统
TopologyMorphSystem morph_system;
SimplicialComplex complex;
// ... 构建复形 ...

// 2. 创建荒谬度场
AbsurdityField field(10, 10, 10);
// ... 设置场值 ...

// 3. 遍历故事节拍
for (const auto& beat : beats) {
    // 4. 演化荒谬度
    SDEEvolutionParams params;
    params.volatility = beat.context.get_volatility();
    params.coupling_strength = beat.context.get_driving_force();
    field.evolve(evolution, params);
    
    // 5. 根据荒谬度触发变形
    if (beat.context.dramatic_tension() > 0.8) {
        morph_system.apply(ChaosMorphRules(), complex, 
                          beat.context.get_driving_force());
    }
}
```

### 示例 3: 多源荒谬度融合

```cpp
#include "cebu/absurdity_fusion.h"

using namespace cebu;

// 创建多源融合器
MultiSourceFusion fusion;

// 来源 1: 叙事驱动
std::vector<FuzzyInterval> narrative_source;
for (const auto& beat : beats) {
    narrative_source.push_back(
        FuzzyInterval(beat.context.surprisal(), 0.1)
    );
}

// 来源 2: 用户交互
std::vector<FuzzyInterval> user_source;
// ... 收集用户数据 ...

// 来源 3: 环境因素
std::vector<FuzzyInterval> env_source;
// ... 收集环境数据 ...

// 加权融合
auto fused = fusion.weighted_average(
    {narrative_source, user_source, env_source},
    {0.5, 0.3, 0.2}
);

// 应用到荒谬度场
field.set_from_fused(fused);
```

---

## 性能特性

| 操作 | Phase 9a | Phase 9b | Phase 9c |
|------|----------|----------|----------|
| FuzzyInterval 运算 | O(1) | - | - |
| SDE 演化步 | O(1) | - | - |
| 荒谬度场查询 | O(log n) | - | - |
| 荒谬度场演化 | O(n) | - | - |
| 多源融合 | O(n·m) | - | - |
| 拓扑变形 | - | O(n log n) | - |
| 叙事分析 | - | - | O(n) |
| 上下文融合 | - | - | O(1) |

---

## 文件清单

### Phase 9a (2,760 行)
- `include/cebu/fuzzy_interval.h` (400 行)
- `include/cebu/absurdity_evolution.h` (300 行)
- `include/cebu/absurdity_fusion.h` (350 行)
- `include/cebu/absurdity_field.h` (450 行)
- `include/cebu/absurdity.h` (300 行)
- `src/fuzzy_interval.cpp` (400 行)
- `src/absurdity_evolution.cpp` (300 行)
- `src/absurdity_fusion.cpp` (400 行)
- `src/absurdity_field.cpp` (500 行)
- `tests/test_phase9a.cpp` (500 行)
- `examples/phase9a_demo.cpp` (600 行)

### Phase 9b (2,540 行)
- `include/cebu/topology_morph.h` (600 行)
- `src/topology_morph.cpp` (900 行)
- `tests/test_phase9b.cpp` (640 行)
- `examples/phase9b_demo.cpp` (400 行)

### Phase 9c (2,000 行)
- `include/cebu/narrative_context.h` (300 行)
- `src/narrative_context.cpp` (600 行)
- `tests/test_phase9c_narrative.cpp` (500 行)
- `examples/phase9c_demo.cpp` (600 行)

**总计**: ~7,300 行代码

---

## 测试覆盖

### Phase 9a (7 个测试)
1. FuzzyInterval 基础
2. SDE 演化
3. 多源融合
4. 荒谬度场
5. 插值和梯度
6. 持久化
7. 荒谬度标签

### Phase 9b (10 个测试)
1. 拓扑变形基础
2. 变形操作
3. 触发条件
4. 预设规则
5. 延迟队列
6. 统计跟踪
7. 事件系统
8. 复杂序列
9. 性能测试
10. 边界条件

### Phase 9c (20 个测试)
1. Narrative Context 基础
2. 驱动力计算
3. 波动率计算
4. 上下文融合
5. 叙事预设
6. 自定义上下文
7. 三幕式结构
8. 英雄之旅
9. 救猫咪
10. 自定义结构
11. 叙事分析器
12. 张力计算
13. 高潮检测
14. 下一事件类型
15. 节奏计算
16. 故事节拍创建
17. 上下文转字符串
18. 高潮强度变化
19. 转折幅度变化
20. 冲突强度变化

**总计**: 37 个测试用例

---

## 参考资料

- [Phase 9a 完成报告](../.codebuddy/phase9a_summary.md)
- [Phase 9b 完成报告](../.codebuddy/phase9b_summary.md)
- [Phase 9c 完成报告](../.codebuddy/phase9c_summary.md)
- [荒谬度需求文档](../prepare/荒谬度.md)
- [Phase 9a 文档](absurdity_system.md)
- [Phase 9c 文档](phase9c_narrative_context.md)
