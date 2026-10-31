# HANDWRITTEN APPENDIX: TASK F BACKPROPAGATION DERIVATION
**Candidate ID**: PASS_RD_2026 | **Module**: Task F | **Max Pages**: 2 Pages

---

## ===================== PAGE 1 / 2 =====================
### 1. PROBLEM SPECIFICATION
- **Given Regression Sample**:
  - Input: $x = 1.0$
  - Target: $y = 2.0$
- **Network Architecture**:
  - Hidden pre-activation: $a = wx + b$
  - Hidden activation: $h = \tanh(a) = \tanh(wx + b)$
  - Output prediction: $\hat{y} = vh + c$
  - Loss Function: $J = \frac{1}{2}(\hat{y} - y)^2$
- **Initial Parameter Values**:
  - $w = 0.5$
  - $b = 0.0$
  - $v = 1.0$
  - $c = 0.0$
  - Learning rate: $\eta = 0.1$

---

### 2. FORWARD PASS EVALUATION
1. **Pre-activation**:
   $$a = w \cdot x + b = (0.5)(1.0) + 0.0 = 0.50000000$$

2. **Hidden Activation**:
   $$h = \tanh(0.50000000) = \frac{e^{0.5} - e^{-0.5}}{e^{0.5} + e^{-0.5}} \approx \mathbf{0.46211716}$$

3. **Output Prediction**:
   $$\hat{y} = v \cdot h + c = (1.0)(0.46211716) + 0.0 = \mathbf{0.46211716}$$

4. **Prediction Error**:
   $$e = \hat{y} - y = 0.46211716 - 2.00000000 = \mathbf{-1.53788284}$$

5. **Loss**:
   $$J = \frac{1}{2}(\hat{y} - y)^2 = \frac{1}{2}(-1.53788284)^2 = \frac{1}{2}(2.36508364) = \mathbf{1.18254182}$$

---

### 3. CHAIN-RULE DERIVATIVES FOR ALL FOUR PARAMETERS
#### A. Derivative with respect to output layer parameters:
$$\frac{\partial J}{\partial \hat{y}} = \frac{\partial}{\partial \hat{y}} \left[ \frac{1}{2}(\hat{y} - y)^2 \right] = (\hat{y} - y) = -1.53788284$$

1. **For parameter $v$**:
   $$\frac{\partial J}{\partial v} = \frac{\partial J}{\partial \hat{y}} \cdot \frac{\partial \hat{y}}{\partial v} = (\hat{y} - y) \cdot h$$
   $$\frac{\partial J}{\partial v} = (-1.53788284) \times (0.46211716) = \mathbf{-0.71068190}$$

2. **For parameter $c$**:
   $$\frac{\partial J}{\partial c} = \frac{\partial J}{\partial \hat{y}} \cdot \frac{\partial \hat{y}}{\partial c} = (\hat{y} - y) \cdot 1.0$$
   $$\frac{\partial J}{\partial c} = (-1.53788284) \times 1.0 = \mathbf{-1.53788284}$$

#### B. Derivative with respect to hidden layer parameters:
$$\frac{\partial J}{\partial h} = \frac{\partial J}{\partial \hat{y}} \cdot \frac{\partial \hat{y}}{\partial h} = (\hat{y} - y) \cdot v = (-1.53788284) \times 1.0 = -1.53788284$$

Derivative of activation $\tanh$:
$$\frac{dh}{da} = 1 - \tanh^2(a) = 1 - h^2 = 1 - (0.46211716)^2 = 1 - 0.21355227 = \mathbf{0.78644773}$$

Backpropagated sensitivity $\frac{\partial J}{\partial a}$:
$$\frac{\partial J}{\partial a} = \frac{\partial J}{\partial h} \cdot \frac{dh}{da} = (-1.53788284) \times (0.78644773) = \mathbf{-1.20946433}$$

3. **For parameter $w$**:
   $$\frac{\partial J}{\partial w} = \frac{\partial J}{\partial a} \cdot \frac{\partial a}{\partial w} = \frac{\partial J}{\partial a} \cdot x$$
   $$\frac{\partial J}{\partial w} = (-1.20946433) \times 1.0 = \mathbf{-1.20946433}$$

4. **For parameter $b$**:
   $$\frac{\partial J}{\partial b} = \frac{\partial J}{\partial a} \cdot \frac{\partial a}{\partial b} = \frac{\partial J}{\partial a} \cdot 1.0$$
   $$\frac{\partial J}{\partial b} = (-1.20946433) \times 1.0 = \mathbf{-1.20946433}$$

---

## ===================== PAGE 2 / 2 =====================
### 4. SIMULTANEOUS GRADIENT DESCENT UPDATE
Using the initial gradients evaluated at $(w^{(0)}, b^{(0)}, v^{(0)}, c^{(0)})$ with learning rate $\eta = 0.1$:
$$\theta^{(1)} = \theta^{(0)} - \eta \cdot \frac{\partial J}{\partial \theta}$$

1. **Update for $w$**:
   $$w^{(1)} = 0.5 - (0.1) \times (-1.20946433) = 0.5 + 0.12094643 = \mathbf{0.62094643}$$

2. **Update for $b$**:
   $$b^{(1)} = 0.0 - (0.1) \times (-1.20946433) = 0.0 + 0.12094643 = \mathbf{0.12094643}$$

3. **Update for $v$**:
   $$v^{(1)} = 1.0 - (0.1) \times (-0.71068190) = 1.0 + 0.07106819 = \mathbf{1.07106819}$$

4. **Update for $c$**:
   $$c^{(1)} = 0.0 - (0.1) \times (-1.53788284) = 0.0 + 0.15378828 = \mathbf{0.15378828}$$

---

### 5. PHYSICAL EXPLANATION & DEFENSE OF GRADIENT DIRECTION
1. **Residual Error Sign Determination**:
   - The initial network prediction is $\hat{y} = 0.4621$, whereas the target is $y = 2.0$.
   - The model is **underpredicting** the target by $1.5379$ units.
   - Hence, the residual error $e = (\hat{y} - y) = -1.5379$ is **negative**.

2. **Sign of All Four Gradients**:
   - Because $h = 0.4621 > 0$, $v = 1.0 > 0$, $(1 - h^2) = 0.7864 > 0$, and $x = 1.0 > 0$, multiplying each by the negative error term $(\hat{y} - y) < 0$ makes **all four analytical gradients strictly negative**:
     $$\frac{\partial J}{\partial w} < 0, \quad \frac{\partial J}{\partial b} < 0, \quad \frac{\partial J}{\partial v} < 0, \quad \frac{\partial J}{\partial c} < 0$$

3. **Direction of Steepest Descent**:
   - In gradient descent optimization, parameters move in the direction opposite to the gradient:
     $$\Delta \theta = -\eta \nabla_\theta J$$
   - Since $\nabla_\theta J < 0$, the update $-\eta \nabla_\theta J > 0$ is **strictly positive**.
   - Every parameter ($w, b, v, c$) increases after the step.

4. **Effect on Prediction and Loss**:
   - Increasing $w$ and $b$ increases the pre-activation $a = wx + b$.
   - Since $\tanh(a)$ is a strictly monotonically increasing function ($\tanh'(a) > 0$), increasing $a$ increases $h$.
   - Since $v > 0$, increasing $h$, $v$, and $c$ directly increases the output prediction:
     $$\hat{y}^{(1)} = v^{(1)} h^{(1)} + c^{(1)} \approx (1.0711)(0.6303) + 0.1538 = \mathbf{0.8289}$$
   - Notice that $\hat{y}$ has moved from $0.4621$ to $0.8289$, closer to $2.0$.
   - The new loss drops from $1.1825$ to $0.6858$, verifying that the gradient descent step moves in the true direction of minimum loss.
