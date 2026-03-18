# Absurdity System API

This document provides a comprehensive reference for Cebu's absurdity system API, which handles fuzzy, interval-based values for narrative metrics and other applications.

## Table of Contents

1. [Core Concepts](#core-concepts)
2. [AbsurdityNumber](#absurditynumber)
3. [AbsurdityOperations](#absurdityoperations)
4. [AbsurdityDistribution](#absurditydistribution)
5. [Advanced Features](#advanced-features)
6. [Code Examples](#code-examples)

---

## Core Concepts

### Absurdity System Components

| Class | Description | Key Features |
|-------|-------------|-------------|
| `AbsurdityNumber` | Represents a fuzzy, interval-based number | Mean, spread, confidence intervals |
| `AbsurdityOperations` | Provides operations for absurdity numbers | Arithmetic, comparison, aggregation |
| `AbsurdityDistribution` | Probability distribution for absurdity numbers | Normal, uniform, custom distributions |

### Key Concepts

- **Mean**: The central value of the absurdity number
- **Spread**: The uncertainty or fuzziness of the value
- **Confidence Interval**: Range of values with a given confidence level
- **Probability Distribution**: How likely different values are

---

## AbsurdityNumber

### Class Definition

```cpp
class AbsurdityNumber {
public:
    // Constructors
    AbsurdityNumber();
    AbsurdityNumber(float mean, float spread);
    AbsurdityNumber(float mean, float spread, std::unique_ptr<AbsurdityDistribution> distribution);
    
    // Core properties
    float get_mean() const;
    float get_spread() const;
    
    // Confidence intervals
    float get_value(float confidence) const;  // 0.0-1.0, lower confidence = wider interval
    float get_lower_bound(float confidence) const;
    float get_upper_bound(float confidence) const;
    
    // Probability
    float get_probability(float value) const;
    float get_probability_range(float min, float max) const;
    
    // Operations
    AbsurdityNumber operator+(const AbsurdityNumber& other) const;
    AbsurdityNumber operator-(const AbsurdityNumber& other) const;
    AbsurdityNumber operator*(const AbsurdityNumber& other) const;
    AbsurdityNumber operator/(const AbsurdityNumber& other) const;
    
    AbsurdityNumber operator+(float scalar) const;
    AbsurdityNumber operator-(float scalar) const;
    AbsurdityNumber operator*(float scalar) const;
    AbsurdityNumber operator/(float scalar) const;
    
    // Comparisons
    bool operator>(const AbsurdityNumber& other) const;
    bool operator<(const AbsurdityNumber& other) const;
    bool operator>=(const AbsurdityNumber& other) const;
    bool operator<=(const AbsurdityNumber& other) const;
    bool operator==(const AbsurdityNumber& other) const;
    
    // Distribution
    const AbsurdityDistribution& get_distribution() const;
    void set_distribution(std::unique_ptr<AbsurdityDistribution> distribution);
};
```

### Constructor Parameters

| Parameter | Description | Default |
|-----------|-------------|---------|
| `mean` | Central value of the absurdity number | `0.0` |
| `spread` | Uncertainty or fuzziness of the value | `0.0` |
| `distribution` | Probability distribution for the value | Normal distribution |

### Methods

#### `get_mean`

```cpp
float get_mean() const;
```

Gets the mean (central value) of the absurdity number.

**Returns:**
- Mean value

#### `get_spread`

```cpp
float get_spread() const;
```

Gets the spread (uncertainty) of the absurdity number.

**Returns:**
- Spread value

#### `get_value`

```cpp
float get_value(float confidence) const;
```

Gets a value from the absurdity number at the given confidence level.

**Parameters:**
- `confidence`: Confidence level (0.0-1.0), where lower values give wider intervals

**Returns:**
- Value at the specified confidence level

#### `get_lower_bound`

```cpp
float get_lower_bound(float confidence) const;
```

Gets the lower bound of the confidence interval.

**Parameters:**
- `confidence`: Confidence level (0.0-1.0)

**Returns:**
- Lower bound of the confidence interval

#### `get_upper_bound`

```cpp
float get_upper_bound(float confidence) const;
```

Gets the upper bound of the confidence interval.

**Parameters:**
- `confidence`: Confidence level (0.0-1.0)

**Returns:**
- Upper bound of the confidence interval

#### `get_probability`

```cpp
float get_probability(float value) const;
```

Gets the probability density at the given value.

**Parameters:**
- `value`: Value to evaluate

**Returns:**
- Probability density at the given value

---

## AbsurdityOperations

### Class Definition

```cpp
class AbsurdityOperations {
public:
    // Arithmetic operations
    static AbsurdityNumber add(const AbsurdityNumber& a, const AbsurdityNumber& b);
    static AbsurdityNumber subtract(const AbsurdityNumber& a, const AbsurdityNumber& b);
    static AbsurdityNumber multiply(const AbsurdityNumber& a, const AbsurdityNumber& b);
    static AbsurdityNumber divide(const AbsurdityNumber& a, const AbsurdityNumber& b);
    
    // Scalar operations
    static AbsurdityNumber add(const AbsurdityNumber& a, float scalar);
    static AbsurdityNumber subtract(const AbsurdityNumber& a, float scalar);
    static AbsurdityNumber multiply(const AbsurdityNumber& a, float scalar);
    static AbsurdityNumber divide(const AbsurdityNumber& a, float scalar);
    
    // Comparison operations
    static bool greater_than(const AbsurdityNumber& a, const AbsurdityNumber& b);
    static bool less_than(const AbsurdityNumber& a, const AbsurdityNumber& b);
    static bool greater_or_equal(const AbsurdityNumber& a, const AbsurdityNumber& b);
    static bool less_or_equal(const AbsurdityNumber& a, const AbsurdityNumber& b);
    static bool equal(const AbsurdityNumber& a, const AbsurdityNumber& b);
    
    // Aggregation
    static AbsurdityNumber average(const std::vector<AbsurdityNumber>& numbers);
    static AbsurdityNumber sum(const std::vector<AbsurdityNumber>& numbers);
    static AbsurdityNumber min(const std::vector<AbsurdityNumber>& numbers);
    static AbsurdityNumber max(const std::vector<AbsurdityNumber>& numbers);
    
    // Utility functions
    static AbsurdityNumber clamp(const AbsurdityNumber& value, float min, float max);
    static AbsurdityNumber lerp(const AbsurdityNumber& a, const AbsurdityNumber& b, float t);
};
```

### Methods

#### `add`

```cpp
static AbsurdityNumber add(const AbsurdityNumber& a, const AbsurdityNumber& b);
```

Adds two absurdity numbers.

**Parameters:**
- `a`: First absurdity number
- `b`: Second absurdity number

**Returns:**
- Sum of the two absurdity numbers

#### `average`

```cpp
static AbsurdityNumber average(const std::vector<AbsurdityNumber>& numbers);
```

Computes the average of a collection of absurdity numbers.

**Parameters:**
- `numbers`: Vector of absurdity numbers

**Returns:**
- Average absurdity number

#### `clamp`

```cpp
static AbsurdityNumber clamp(const AbsurdityNumber& value, float min, float max);
```

Clamps an absurdity number to a range.

**Parameters:**
- `value`: Absurdity number to clamp
- `min`: Minimum value
- `max`: Maximum value

**Returns:**
- Clamped absurdity number

---

## AbsurdityDistribution

### Class Definition

```cpp
class AbsurdityDistribution {
public:
    virtual ~AbsurdityDistribution() = default;
    
    // Probability density function
    virtual float pdf(float value, float mean, float spread) const = 0;
    
    // Cumulative distribution function
    virtual float cdf(float value, float mean, float spread) const = 0;
    
    // Inverse cumulative distribution function
    virtual float icdf(float probability, float mean, float spread) const = 0;
    
    // Sample from the distribution
    virtual float sample(float mean, float spread) const = 0;
};
```

### Concrete Distributions

#### NormalDistribution

```cpp
class NormalDistribution : public AbsurdityDistribution {
public:
    float pdf(float value, float mean, float spread) const override;
    float cdf(float value, float mean, float spread) const override;
    float icdf(float probability, float mean, float spread) const override;
    float sample(float mean, float spread) const override;
};
```

#### UniformDistribution

```cpp
class UniformDistribution : public AbsurdityDistribution {
public:
    float pdf(float value, float mean, float spread) const override;
    float cdf(float value, float mean, float spread) const override;
    float icdf(float probability, float mean, float spread) const override;
    float sample(float mean, float spread) const override;
};
```

#### TriangularDistribution

```cpp
class TriangularDistribution : public AbsurdityDistribution {
public:
    float pdf(float value, float mean, float spread) const override;
    float cdf(float value, float mean, float spread) const override;
    float icdf(float probability, float mean, float spread) const override;
    float sample(float mean, float spread) const override;
};
```

---

## Advanced Features

### Custom Distributions

You can create custom probability distributions by extending the `AbsurdityDistribution` class:

```cpp
class ExponentialDistribution : public cebu::AbsurdityDistribution {
public:
    float pdf(float value, float mean, float spread) const override {
        if (value < 0) return 0.0;
        float lambda = 1.0 / mean;
        return lambda * std::exp(-lambda * value);
    }
    
    float cdf(float value, float mean, float spread) const override {
        if (value < 0) return 0.0;
        float lambda = 1.0 / mean;
        return 1.0 - std::exp(-lambda * value);
    }
    
    float icdf(float probability, float mean, float spread) const override {
        if (probability <= 0) return 0.0;
        if (probability >= 1) return std::numeric_limits<float>::infinity();
        float lambda = 1.0 / mean;
        return -std::log(1.0 - probability) / lambda;
    }
    
    float sample(float mean, float spread) const override {
        float u = static_cast<float>(std::rand()) / RAND_MAX;
        return icdf(u, mean, spread);
    }
};

// Usage
cebu::AbsurdityNumber custom_number(10.0, 2.0, std::make_unique<ExponentialDistribution>());
```

### Monte Carlo Sampling

You can use Monte Carlo sampling to estimate properties of absurdity numbers:

```cpp
// Monte Carlo sampling example
cebu::AbsurdityNumber tension(0.5, 0.2);
int samples = 10000;
float sum = 0.0;

for (int i = 0; i < samples; ++i) {
    float sample = tension.get_distribution().sample(tension.get_mean(), tension.get_spread());
    sum += sample;
}

float estimated_mean = sum / samples;
std::cout << "Estimated mean: " << estimated_mean << std::endl;
std::cout << "Actual mean: " << tension.get_mean() << std::endl;
```

### Confidence Intervals

You can work with confidence intervals to understand the uncertainty in absurdity numbers:

```cpp
cebu::AbsurdityNumber temperature(25.0, 2.0);

// Get 95% confidence interval
float lower = temperature.get_lower_bound(0.95);
float upper = temperature.get_upper_bound(0.95);
std::cout << "95% confidence interval: [" << lower << ", " << upper << "]" << std::endl;

// Get 50% confidence interval
lower = temperature.get_lower_bound(0.5);
upper = temperature.get_upper_bound(0.5);
std::cout << "50% confidence interval: [" << lower << ", " << upper << "]" << std::endl;
```

---

## Code Examples

### Basic Usage

```cpp
#include "cebu/absurdity/absurdity_number.h"
#include <iostream>

int main() {
    using namespace cebu;
    
    // Create an absurdity number
    AbsurdityNumber temp(20.0, 5.0);  // Mean 20, spread 5
    
    // Get mean and spread
    std::cout << "Mean: " << temp.get_mean() << std::endl;
    std::cout << "Spread: " << temp.get_spread() << std::endl;
    
    // Get confidence intervals
    std::cout << "\nConfidence intervals:" << std::endl;
    std::cout << "90%: [" << temp.get_lower_bound(0.9) << ", " << temp.get_upper_bound(0.9) << "]" << std::endl;
    std::cout << "50%: [" << temp.get_lower_bound(0.5) << ", " << temp.get_upper_bound(0.5) << "]" << std::endl;
    
    // Get values at different confidence levels
    std::cout << "\nValues at different confidence levels:" << std::endl;
    std::cout << "0.1 (high): " << temp.get_value(0.1) << std::endl;
    std::cout << "0.5 (mean): " << temp.get_value(0.5) << std::endl;
    std::cout << "0.9 (low): " << temp.get_value(0.9) << std::endl;
    
    // Get probabilities
    std::cout << "\nProbabilities:" << std::endl;
    std::cout << "Probability at 20: " << temp.get_probability(20.0) << std::endl;
    std::cout << "Probability at 25: " << temp.get_probability(25.0) << std::endl;
    std::cout << "Probability between 15 and 25: " << temp.get_probability_range(15.0, 25.0) << std::endl;
    
    return 0;
}
```

### Arithmetic Operations

```cpp
#include "cebu/absurdity/absurdity_number.h"
#include "cebu/absurdity/absurdity_operations.h"
#include <iostream>

int main() {
    using namespace cebu;
    
    // Create two absurdity numbers
    AbsurdityNumber a(10.0, 2.0);
    AbsurdityNumber b(5.0, 1.0);
    
    // Addition
    AbsurdityNumber sum = a + b;
    std::cout << "Sum: mean=" << sum.get_mean() << ", spread=" << sum.get_spread() << std::endl;
    
    // Subtraction
    AbsurdityNumber diff = a - b;
    std::cout << "Difference: mean=" << diff.get_mean() << ", spread=" << diff.get_spread() << std::endl;
    
    // Multiplication
    AbsurdityNumber product = a * b;
    std::cout << "Product: mean=" << product.get_mean() << ", spread=" << product.get_spread() << std::endl;
    
    // Division
    AbsurdityNumber quotient = a / b;
    std::cout << "Quotient: mean=" << quotient.get_mean() << ", spread=" << quotient.get_spread() << std::endl;
    
    // Scalar operations
    AbsurdityNumber scaled = a * 2.0;
    std::cout << "Scaled by 2: mean=" << scaled.get_mean() << ", spread=" << scaled.get_spread() << std::endl;
    
    return 0;
}
```

### Comparisons

```cpp
#include "cebu/absurdity/absurdity_number.h"
#include <iostream>

int main() {
    using namespace cebu;
    
    // Create absurdity numbers
    AbsurdityNumber a(10.0, 2.0);
    AbsurdityNumber b(15.0, 2.0);
    AbsurdityNumber c(10.0, 1.0);
    
    // Comparisons
    std::cout << "a > b: " << (a > b) << std::endl;  // Should be false
    std::cout << "b > a: " << (b > a) << std::endl;  // Should be true
    std::cout << "a == c: " << (a == c) << std::endl;  // Should be true
    std::cout << "a < c: " << (a < c) << std::endl;  // Should be false
    
    return 0;
}
```

### Aggregation

```cpp
#include "cebu/absurdity/absurdity_number.h"
#include "cebu/absurdity/absurdity_operations.h"
#include <iostream>
#include <vector>

int main() {
    using namespace cebu;
    
    // Create a vector of absurdity numbers
    std::vector<AbsurdityNumber> numbers;
    numbers.emplace_back(10.0, 2.0);
    numbers.emplace_back(15.0, 1.0);
    numbers.emplace_back(8.0, 1.5);
    numbers.emplace_back(12.0, 2.5);
    
    // Compute average
    AbsurdityNumber avg = AbsurdityOperations::average(numbers);
    std::cout << "Average: mean=" << avg.get_mean() << ", spread=" << avg.get_spread() << std::endl;
    
    // Compute sum
    AbsurdityNumber sum = AbsurdityOperations::sum(numbers);
    std::cout << "Sum: mean=" << sum.get_mean() << ", spread=" << sum.get_spread() << std::endl;
    
    // Compute min and max
    AbsurdityNumber min_val = AbsurdityOperations::min(numbers);
    AbsurdityNumber max_val = AbsurdityOperations::max(numbers);
    std::cout << "Min: mean=" << min_val.get_mean() << ", spread=" << min_val.get_spread() << std::endl;
    std::cout << "Max: mean=" << max_val.get_mean() << ", spread=" << max_val.get_spread() << std::endl;
    
    return 0;
}
```

### Narrative Metrics Example

```cpp
#include "cebu/absurdity/absurdity_number.h"
#include "cebu/narrative/narrative_context.h"
#include <iostream>

int main() {
    using namespace cebu;
    
    // Create narrative context
    NarrativeContext context;
    
    // Set absurdity numbers for narrative metrics
    context.set_value("tension", AbsurdityNumber(0.5, 0.2));
    context.set_value("excitement", AbsurdityNumber(0.3, 0.15));
    context.set_value("drama", AbsurdityNumber(0.4, 0.25));
    
    // Retrieve and update
    auto tension = context.get_value<AbsurdityNumber>("tension");
    tension += 0.2;  // Increase tension
    context.set_value("tension", tension);
    
    // Get updated value
    tension = context.get_value<AbsurdityNumber>("tension");
    std::cout << "Updated tension: mean=" << tension.get_mean() << ", spread=" << tension.get_spread() << std::endl;
    
    // Check if tension is high
    AbsurdityNumber high_tension_threshold(0.7, 0.1);
    if (tension > high_tension_threshold) {
        std::cout << "High tension detected!" << std::endl;
    }
    
    // Get confidence interval for excitement
    auto excitement = context.get_value<AbsurdityNumber>("excitement");
    float lower = excitement.get_lower_bound(0.9);
    float upper = excitement.get_upper_bound(0.9);
    std::cout << "90% confidence interval for excitement: [" << lower << ", " << upper << "]" << std::endl;
    
    return 0;
}
```

---

**Cebu Version**: 0.8.0
**Last Updated**: 2026-03-18