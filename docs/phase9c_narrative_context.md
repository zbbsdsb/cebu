# Phase 9c: Narrative Context - 叙事上下文系统

## 概述

Phase 9c 实现了完整的叙事上下文系统，将荒谬度演化与戏剧叙事结构深度集成。通过提供**意外性 (S)**、**逻辑偏离度 (L)**、**情绪强度**、**戏剧张力**等关键参数，驱动 Phase 9a 的荒谬度随机演化方程 (SDE)。

## 核心概念

### 叙事上下文

叙事上下文描述故事在特定时刻的戏剧状态，包含以下核心维度：

- **意外性 (Surprisal, S)**: `[0, 1]` - 情境的意外程度
- **逻辑偏离度 (Logic Deviation, L)**: `[0, 1]` - 偏离逻辑一致性的程度
- **情绪强度 (Emotional Intensity, E)**: `[0, 1]` - 当前情绪状态
- **戏剧张力 (Dramatic Tension, T)**: `[0, 1]` - 叙事张力水平
- **时间距离 (Time Distance, D)**: `[0, 1]` - 距离高潮的时间 (0 = 高潮, 1 = 开始)

### 与荒谬度演化的集成

```cpp
NarrativeContext ctx(0.8, 0.7, 0.9, 0.9, 0.0);

// 驱动荒谬度 SDE 方程
params.volatility = ctx.get_volatility();           // σ = σ₀ · (1 + E) · (1 + T)
params.coupling_strength = ctx.get_driving_force(); // κ·S + η·L

// 演化荒谬度
auto new_absurdity = evolution.step(current, params);
```

## API 参考

### `NarrativeContext`

#### 构造函数

```cpp
NarrativeContext(
    double surprisal = 0.0,              // 意外性 S
    double logic_deviation = 0.0,        // 逻辑偏离度 L
    double emotional_intensity = 0.0,     // 情绪强度 E
    double dramatic_tension = 0.0,       // 戏剧张力 T
    double time_distance = 1.0           // 时间距离 D
);
```

#### 核心方法

```cpp
// 获取 SDE 驱动项：κ·S + η·L
double get_driving_force(double kappa = 1.0, double eta = 1.0) const;

// 获取波动率：σ₀ · (1 + E) · (1 + T)
double get_volatility(double base_volatility = 0.1) const;

// 与另一个上下文融合
NarrativeContext fuse(const NarrativeContext& other, double weight = 0.5) const;

// 转换为字符串
std::string to_string() const;
```

#### Getter 方法

```cpp
double surprisal() const;              // 意外性 S
double logic_deviation() const;        // 逻辑偏离度 L
double emotional_intensity() const;     // 情绪强度 E
double dramatic_tension() const;        // 戏剧张力 T
double time_distance() const;          // 时间距离 D
```

---

### `DramaticEventType`

戏剧事件类型枚举，共 17 种：

#### 经典事件 (9 种)

| 事件类型 | 枚举值 | 描述 |
|---------|-------|------|
| Exposition | `EXPOSITION` | 铺垫：介绍背景和角色 |
| Inciting Incident | `INCITING_INCIDENT` | 激励事件：打破平衡 |
| Rising Action | `RISING_ACTION` | 上升动作：冲突升级 |
| Plot Point 1 | `PLOT_POINT_1` | 第一个转折点：进入第二幕 |
| Midpoint | `MIDPOINT` | 中点：方向改变 |
| Plot Point 2 | `PLOT_POINT_2` | 第二个转折点：进入第三幕 |
| Climax | `CLIMAX` | 高潮：最终对决 |
| Falling Action | `FALLING_ACTION` | 下降动作：冲突解决 |
| Resolution | `RESOLUTION` | 结局：新的平衡 |

#### 特殊事件 (8 种)

| 事件类型 | 枚举值 | 描述 |
|---------|-------|------|
| Twist | `TWIST` | 剧情转折：出人意料的反转 |
| Revelation | `REVELATION` | 揭秘：重要信息揭示 |
| Conflict | `CONFLICT` | 冲突：对立面交锋 |
| Transition | `TRANSITION` | 过渡：场景/时间切换 |
| Pause | `PAUSE` | 暂停：节奏放慢 |
| Flashback | `FLASHBACK` | 闪回：过去场景 |
| Flashforward | `FLASHFORWARD` | 闪前：未来场景 |
| Montage | `MONTAGE` | 蒙太奇：快速剪辑 |
| Time Skip | `TIME_SKIP` | 时间跳转 |
| Custom | `CUSTOM` | 自定义事件 |

---

### `StoryBeat`

故事节拍结构，包含事件类型和对应的叙事上下文：

```cpp
struct StoryBeat {
    DramaticEventType type;    // 事件类型
    NarrativeContext context;  // 叙事上下文
    
    // 构造函数
    StoryBeat(DramaticEventType type, const NarrativeContext& context);
    
    // 转换为字符串
    std::string to_string() const;
};
```

---

### `NarrativePresets`

预设叙事上下文工厂，为所有事件类型提供标准配置。

#### 方法

```cpp
// 获取特定事件类型的预设上下文
static NarrativeContext get_preset(DramaticEventType type);

// 可变强度的预设
static NarrativeContext climax(double intensity = 1.0);
static NarrativeContext twist(double magnitude = 0.8);
static NarrativeContext conflict(double intensity = 0.7);

// 闪回和闪前
static NarrativeContext flashback(double intensity = 0.5, double distance = 0.5);
static NarrativeContext flashforward(double intensity = 0.5, double distance = 0.5);

// 时间跳转
static NarrativeContext time_skip(double distance = 0.3);

// 完全自定义
static NarrativeContext custom(
    double surprisal,
    double logic_deviation,
    double emotional_intensity = 0.0,
    double dramatic_tension = 0.0,
    double time_distance = 0.0
);
```

#### 预设参数表

| 事件类型 | S | L | E | T | D | 描述 |
|---------|---|---|---|---|---|------|
| Exposition | 0.1 | 0.0 | 0.2 | 0.1 | 1.0 | 平静介绍 |
| Inciting Incident | 0.7 | 0.3 | 0.5 | 0.4 | 0.9 | 激励事件 |
| Rising Action | 0.4 | 0.2 | 0.4 | 0.5 | 0.7 | 上升动作 |
| Plot Point 1 | 0.6 | 0.5 | 0.6 | 0.7 | 0.6 | 第一转折 |
| Midpoint | 0.5 | 0.3 | 0.7 | 0.6 | 0.5 | 中点 |
| Plot Point 2 | 0.7 | 0.6 | 0.8 | 0.8 | 0.4 | 第二转折 |
| Climax | 1.0 | 0.8 | 1.0 | 1.0 | 0.0 | 高潮 |
| Falling Action | 0.3 | 0.2 | 0.4 | 0.3 | 0.2 | 下降动作 |
| Resolution | 0.1 | 0.0 | 0.2 | 0.1 | 0.0 | 结局 |
| Twist | 0.9 | 0.9 | 0.8 | 0.9 | 0.5 | 剧情转折 |
| Revelation | 0.8 | 0.4 | 0.7 | 0.6 | 0.5 | 揭秘 |
| Conflict | 0.6 | 0.3 | 0.7 | 0.8 | 0.5 | 冲突 |
| Transition | 0.3 | 0.2 | 0.3 | 0.3 | 0.5 | 过渡 |
| Pause | 0.2 | 0.1 | 0.2 | 0.1 | 0.5 | 暂停 |
| Flashback | 0.4 | 0.3 | 0.5 | 0.4 | 0.5 | 闪回 |
| Flashforward | 0.5 | 0.4 | 0.6 | 0.5 | 0.5 | 闪前 |
| Montage | 0.4 | 0.2 | 0.5 | 0.5 | 0.5 | 蒙太奇 |
| Time Skip | 0.6 | 0.3 | 0.4 | 0.3 | 0.5 | 时间跳转 |
| Custom | 可变 | 可变 | 可变 | 可变 | 可变 | 自定义 |

---

### `NarrativeStructure`

叙事结构类型枚举：

```cpp
enum class NarrativeStructure {
    THREE_ACT,         // 三幕式结构
    HEROS_JOURNEY,     // 英雄之旅
    SAVE_THE_CAT,      // 救猫咪
    CUSTOM             // 自定义结构
};
```

---

### `NarrativeTemplate`

叙事模板，提供标准的故事结构节拍序列。

#### 方法

```cpp
// 获取特定结构的节拍序列
static std::vector<StoryBeat> get_template(NarrativeStructure type);

// 三幕式结构 (9 个节拍)
static std::vector<StoryBeat> three_act_structure();

// 英雄之旅 (12 个节拍)
static std::vector<StoryBeat> heros_journey();

// 救猫咪 (15 个节拍)
static std::vector<StoryBeat> save_the_cat();
```

#### 三幕式结构节拍

```cpp
{
    {EXPOSITION, {0.1, 0.0, 0.2, 0.1, 1.0}},       // Act 1: Setup
    {INCITING_INCIDENT, {0.7, 0.3, 0.5, 0.4, 0.9}},
    {RISING_ACTION, {0.4, 0.2, 0.4, 0.5, 0.7}},
    {PLOT_POINT_1, {0.6, 0.5, 0.6, 0.7, 0.6}},    // Act 2: Confrontation
    {MIDPOINT, {0.5, 0.3, 0.7, 0.6, 0.5}},
    {RISING_ACTION, {0.4, 0.2, 0.5, 0.7, 0.4}},
    {PLOT_POINT_2, {0.7, 0.6, 0.8, 0.8, 0.4}},    // Act 3: Resolution
    {CLIMAX, {1.0, 0.8, 1.0, 1.0, 0.0}},
    {FALLING_ACTION, {0.3, 0.2, 0.4, 0.3, 0.2}},
    {RESOLUTION, {0.1, 0.0, 0.2, 0.1, 0.0}}
}
```

---

### `NarrativeAnalyzer`

动态叙事分析器，基于历史和未来节拍分析当前状态。

#### 构造函数

```cpp
NarrativeAnalyzer(const std::vector<StoryBeat>& beats);
```

#### 方法

```cpp
// 分析当前位置的叙事状态
NarrativeContext analyze_at(size_t position) const;

// 计算叙事张力
double calculate_tension() const;

// 检测是否接近高潮
bool is_approaching_climax(size_t position, double threshold = 3) const;

// 预测下一个事件类型
DramaticEventType predict_next_type(size_t position) const;

// 计算节奏（事件密度）
double calculate_rhythm(size_t position, double window = 5) const;
```

---

## 使用示例

### 基础使用

```cpp
#include "cebu/narrative_context.h"

using namespace cebu;

// 创建叙事上下文
NarrativeContext ctx(0.8, 0.7, 0.9, 0.9, 0.0);

// 获取驱动力和波动率
double driving_force = ctx.get_driving_force();      // κ·S + η·L
double volatility = ctx.get_volatility();            // σ₀ · (1 + E) · (1 + T)

std::cout << "Driving Force: " << driving_force << "\n";
std::cout << "Volatility: " << volatility << "\n";
std::cout << "Context: " << ctx.to_string() << "\n";
```

### 使用预设

```cpp
// 使用预设上下文
auto climax_ctx = NarrativePresets::climax(1.0);
auto twist_ctx = NarrativePresets::twist(0.8);
auto conflict_ctx = NarrativePresets::conflict(0.7);

// 自定义上下文
auto custom_ctx = NarrativePresets::custom(
    0.9, 0.8, 0.7, 0.6, 0.3
);
```

### 上下文融合

```cpp
NarrativeContext ctx1(0.8, 0.7, 0.9, 0.9, 0.0);
NarrativeContext ctx2(0.6, 0.5, 0.7, 0.8, 0.2);

// 融合两个上下文（权重 0.5）
auto fused = ctx1.fuse(ctx2, 0.5);
```

### 创建故事节拍

```cpp
// 创建故事节拍
StoryBeat beat(DramaticEventType::CLIMAX, 
               NarrativePresets::climax(1.0));

std::cout << beat.to_string() << "\n";
// 输出: Climax: S=1.0, L=0.8, E=1.0, T=1.0, D=0.0
```

### 使用叙事模板

```cpp
// 获取三幕式结构
auto beats = NarrativeTemplate::three_act_structure();

// 遍历节拍
for (size_t i = 0; i < beats.size(); ++i) {
    std::cout << "Beat " << i << ": " << beats[i].to_string() << "\n";
}

// 获取英雄之旅
auto hero_journey = NarrativeTemplate::heros_journey();
```

### 叙事分析

```cpp
// 创建分析器
NarrativeAnalyzer analyzer(beats);

// 分析特定位置的状态
auto ctx = analyzer.analyze_at(5);
std::cout << "Analysis: " << ctx.to_string() << "\n";

// 检查是否接近高潮
if (analyzer.is_approaching_climax(5)) {
    std::cout << "Approaching climax!\n";
}

// 预测下一个事件类型
auto next_type = analyzer.predict_next_type(5);
std::cout << "Next event: " << static_cast<int>(next_type) << "\n";

// 计算节奏
double rhythm = analyzer.calculate_rhythm(5);
std::cout << "Rhythm: " << rhythm << "\n";
```

### 与荒谬度系统集成

```cpp
#include "cebu/absurdity_evolution.h"

// 创建叙事上下文
NarrativeContext ctx(0.8, 0.7, 0.9, 0.9, 0.0);

// 配置荒谬度演化参数
SDEEvolutionParams params;
params.volatility = ctx.get_volatility();                    // σ
params.coupling_strength = ctx.get_driving_force();          // κ·S + η·L
params.diffusion_strength = 0.1;
params.time_step = 0.01;

// 创建演化器
SDEEvolution evolution;

// 演化荒谬度
FuzzyInterval current(0.5, 0.1);
auto evolved = evolution.step(current, params);
```

### 与拓扑变形集成

```cpp
#include "cebu/topology_morph.h"

// 创建拓扑变形系统
TopologyMorphSystem morph_system;

// 创建故事节拍
StoryBeat beat(DramaticEventType::CLIMAX, 
               NarrativePresets::climax(1.0));

// 根据戏剧张力触发变形
if (beat.context.dramatic_tension() > 0.8) {
    morph_system.apply(
        ChaosMorphRules(),
        complex,
        beat.context.get_driving_force()
    );
}
```

---

## 完整演示

### 演示 1: 荒谬度与叙事演化

```cpp
#include "cebu/narrative_context.h"
#include "cebu/absurdity_evolution.h"

using namespace cebu;

int main() {
    // 创建三幕式结构
    auto beats = NarrativeTemplate::three_act_structure();
    
    // 初始化荒谬度
    FuzzyInterval absurdity(0.5, 0.1);
    SDEEvolution evolution;
    
    std::cout << "=== 荒谬度与叙事演化 ===\n\n";
    
    // 遍历故事节拍
    for (size_t i = 0; i < beats.size(); ++i) {
        const auto& beat = beats[i];
        
        std::cout << "Beat " << i << ": " << beat.to_string() << "\n";
        
        // 配置演化参数
        SDEEvolutionParams params;
        params.volatility = beat.context.get_volatility();
        params.coupling_strength = beat.context.get_driving_force();
        params.diffusion_strength = 0.1;
        params.time_step = 0.01;
        
        // 演化荒谬度
        absurdity = evolution.step(absurdity, params);
        
        std::cout << "  → Absurdity: " << absurdity.to_string() << "\n";
        std::cout << "  → Volatility: " << params.volatility << "\n";
        std::cout << "  → Driving Force: " << params.coupling_strength << "\n\n";
    }
    
    return 0;
}
```

---

## 测试

运行测试套件：

```bash
./build/test_phase9c_narrative
```

测试覆盖：
- Narrative Context 基础功能
- 驱动力和波动率计算
- 上下文融合
- 叙事预设
- 叙事结构模板
- 叙事分析器

---

## 性能特性

| 操作 | 时间复杂度 | 空间复杂度 |
|------|-----------|-----------|
| 创建上下文 | O(1) | O(1) |
| 获取驱动力/波动率 | O(1) | O(1) |
| 上下文融合 | O(1) | O(1) |
| 获取模板 | O(n) | O(n) |
| 叙事分析 | O(n) | O(n) |

---

## 相关模块

- **Phase 9a**: 荒谬度系统 (`absurdity.h`)
- **Phase 9b**: 拓扑变形 (`topology_morph.h`)
- **叙事系统**: `story_event.h`, `simplicial_complex_narrative.h`, `timeline.h`

---

## 参考资料

- [荒谬度需求文档](../prepare/荒谬度.md)
- [Phase 9a 完成](.codebuddy/phase9a_summary.md)
- [Phase 9b 完成](.codebuddy/phase9b_summary.md)
