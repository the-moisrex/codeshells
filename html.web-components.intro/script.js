this.HTMLElement.prototype.makeComponent = function() {
    this.attachShadow({mode: 'open'});
    let self = this;
    const template = document.createElement('template');
    const proxy = new Proxy(template, {
        get() {
            const res = Reflect.get(...arguments);
            return res;
        },
        set(target, prop, value) {
            target[prop] = value;
            if (prop == 'innerHTML') {
                self.shadowRoot.innerHTML = '';
                self.shadowRoot.append(template.content.cloneNode(true));
            }
            return true;
        }
    });
    Object.defineProperty(this, "template", {
        get() {
            return proxy;
        }
    });

    this.attributeChcangedCallback = (name, old, val) => {
        self[name] = val;
    };
};


class TodoItem extends HTMLElement {

    constructor() {
        super();
        this.makeComponent();
        this.template.innerHTML = `
            <style>
                label {
                    display: block;
                    padding: 0 3em;
                    font-family: monospace;
                }
                input {
                    display: none;
                }
                label::before {
                    content: "[ ] ";
                }
                label:has(input:checked)::before {
                    content: "[X] ";
                }
            </style>
            <label>
                <input type="checkbox" />
                <slot></slot>
            </label>
        `;
    }

    static get observedAttributes() {
        return ["checked"];
    }
}

customElements.define('todo-item', TodoItem);

document.querySelector('todo-item').checked = true;
