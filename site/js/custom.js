// Custom JavaScript for Cebu Documentation

// Wait for DOM to be fully loaded
document.addEventListener('DOMContentLoaded', function() {
  // Add smooth scrolling to anchor links
  document.querySelectorAll('a[href^="#"]').forEach(anchor => {
    anchor.addEventListener('click', function (e) {
      e.preventDefault();
      
      const targetId = this.getAttribute('href');
      if (targetId === '#') return;
      
      const targetElement = document.querySelector(targetId);
      if (targetElement) {
        targetElement.scrollIntoView({
          behavior: 'smooth',
          block: 'start'
        });
      }
    });
  });

  // Add copy button to code blocks
  document.querySelectorAll('pre code').forEach((codeBlock, index) => {
    const pre = codeBlock.parentElement;
    
    // Skip if already has a copy button
    if (pre.querySelector('.copy-button')) return;
    
    const button = document.createElement('button');
    button.className = 'copy-button';
    button.textContent = 'Copy';
    button.title = 'Copy code to clipboard';
    
    // Style the button
    button.style.position = 'absolute';
    button.style.top = '0.5rem';
    button.style.right = '0.5rem';
    button.style.padding = '0.25rem 0.5rem';
    button.style.fontSize = '0.75rem';
    button.style.backgroundColor = 'var(--md-primary-fg-color)';
    button.style.color = 'white';
    button.style.border = 'none';
    button.style.borderRadius = '4px';
    button.style.cursor = 'pointer';
    button.style.opacity = '0.7';
    button.style.transition = 'opacity 0.2s ease-in-out';
    
    // Add hover effect
    button.addEventListener('mouseover', function() {
      this.style.opacity = '1';
    });
    
    button.addEventListener('mouseout', function() {
      this.style.opacity = '0.7';
    });
    
    // Add click event to copy code
    button.addEventListener('click', function() {
      const code = codeBlock.textContent;
      navigator.clipboard.writeText(code).then(() => {
        const originalText = button.textContent;
        button.textContent = 'Copied!';
        button.style.backgroundColor = 'var(--md-accent-fg-color)';
        
        setTimeout(() => {
          button.textContent = originalText;
          button.style.backgroundColor = 'var(--md-primary-fg-color)';
        }, 2000);
      });
    });
    
    // Make pre element position relative
    pre.style.position = 'relative';
    
    // Append button to pre element
    pre.appendChild(button);
  });

  // Add scroll spy for navigation
  const sections = document.querySelectorAll('section[id]');
  const navLinks = document.querySelectorAll('.md-nav__link');
  
  function highlightNavLink() {
    let current = '';
    
    sections.forEach(section => {
      const sectionTop = section.offsetTop;
      const sectionHeight = section.clientHeight;
      if (window.pageYOffset >= sectionTop - 100) {
        current = section.getAttribute('id');
      }
    });
    
    navLinks.forEach(link => {
      link.classList.remove('active');
      if (link.getAttribute('href').substring(1) === current) {
        link.classList.add('active');
      }
    });
  }
  
  window.addEventListener('scroll', highlightNavLink);
  
  // Add animation to feature boxes
  const featureBoxes = document.querySelectorAll('.feature-box');
  
  function checkFeatureBoxes() {
    const triggerBottom = window.innerHeight * 0.8;
    
    featureBoxes.forEach(box => {
      const boxTop = box.getBoundingClientRect().top;
      
      if (boxTop < triggerBottom) {
        box.style.opacity = '1';
        box.style.transform = 'translateY(0)';
      }
    });
  }
  
  // Initial check
  checkFeatureBoxes();
  
  // Check on scroll
  window.addEventListener('scroll', checkFeatureBoxes);
  
  // Add dark mode toggle
  const darkModeToggle = document.querySelector('.md-header__button.md-icon--dark');
  if (darkModeToggle) {
    darkModeToggle.addEventListener('click', function() {
      document.body.classList.toggle('dark-mode');
    });
  }
  
  // Add search functionality enhancement
  const searchInput = document.querySelector('.md-search__input');
  if (searchInput) {
    searchInput.addEventListener('focus', function() {
      this.parentElement.classList.add('search-focused');
    });
    
    searchInput.addEventListener('blur', function() {
      this.parentElement.classList.remove('search-focused');
    });
  }
  
  // Add table of contents toggle
  const tocToggle = document.querySelector('.md-toc__title');
  if (tocToggle) {
    tocToggle.addEventListener('click', function() {
      this.parentElement.classList.toggle('toc-open');
    });
  }
  
  // Add responsive menu toggle
  const menuToggle = document.querySelector('.md-header__button.md-icon--menu');
  if (menuToggle) {
    menuToggle.addEventListener('click', function() {
      document.body.classList.toggle('menu-open');
    });
  }
  
  // Add back to top button functionality
  const backToTopButton = document.querySelector('.md-top');
  if (backToTopButton) {
    window.addEventListener('scroll', function() {
      if (window.pageYOffset > 300) {
        backToTopButton.style.opacity = '1';
        backToTopButton.style.visibility = 'visible';
      } else {
        backToTopButton.style.opacity = '0';
        backToTopButton.style.visibility = 'hidden';
      }
    });
  }
  
  // Add code syntax highlighting toggle
  const codeBlocks = document.querySelectorAll('pre code');
  codeBlocks.forEach(block => {
    // Add language class if not present
    if (!block.classList.contains('language-')) {
      block.classList.add('language-cpp');
    }
  });
  
  // Add tab functionality for code examples
  const tabbedSets = document.querySelectorAll('.tabbed-set');
  tabbedSets.forEach(set => {
    const tabs = set.querySelectorAll('.tabbed-set__tab');
    tabs.forEach(tab => {
      tab.addEventListener('click', function() {
        // Remove active class from all tabs
        tabs.forEach(t => t.classList.remove('active'));
        // Add active class to clicked tab
        this.classList.add('active');
        
        // Get the tab content
        const tabId = this.getAttribute('data-tab');
        const tabContents = set.querySelectorAll('.tabbed-content');
        
        // Hide all tab contents
        tabContents.forEach(content => {
          content.style.display = 'none';
        });
        
        // Show the selected tab content
        const selectedContent = set.querySelector(`.tabbed-content[data-tab="${tabId}"]`);
        if (selectedContent) {
          selectedContent.style.display = 'block';
        }
      });
    });
    
    // Activate first tab by default
    if (tabs.length > 0) {
      tabs[0].click();
    }
  });
  
  // Add smooth animation to page load
  document.body.style.opacity = '0';
  document.body.style.transition = 'opacity 0.5s ease-in-out';
  
  setTimeout(() => {
    document.body.style.opacity = '1';
  }, 100);
});

// Add custom event for documentation feedback
function sendFeedback(type, message) {
  const feedbackData = {
    type: type,
    message: message,
    page: window.location.href,
    timestamp: new Date().toISOString()
  };
  
  // In a real implementation, this would send data to a server
  console.log('Feedback:', feedbackData);
  
  // Show feedback confirmation
  const feedbackElement = document.createElement('div');
  feedbackElement.className = 'feedback-confirmation';
  feedbackElement.textContent = 'Thank you for your feedback!';
  feedbackElement.style.position = 'fixed';
  feedbackElement.style.bottom = '20px';
  feedbackElement.style.right = '20px';
  feedbackElement.style.padding = '1rem';
  feedbackElement.style.backgroundColor = 'var(--md-accent-fg-color)';
  feedbackElement.style.color = 'white';
  feedbackElement.style.borderRadius = '6px';
  feedbackElement.style.zIndex = '9999';
  feedbackElement.style.boxShadow = '0 4px 12px rgba(0, 0, 0, 0.15)';
  feedbackElement.style.animation = 'slideIn 0.3s ease-out';
  
  document.body.appendChild(feedbackElement);
  
  setTimeout(() => {
    feedbackElement.style.animation = 'slideOut 0.3s ease-in';
    setTimeout(() => {
      document.body.removeChild(feedbackElement);
    }, 300);
  }, 3000);
}

// Add CSS animations
const style = document.createElement('style');
style.textContent = `
  @keyframes slideIn {
    from {
      transform: translateX(100%);
      opacity: 0;
    }
    to {
      transform: translateX(0);
      opacity: 1;
    }
  }
  
  @keyframes slideOut {
    from {
      transform: translateX(0);
      opacity: 1;
    }
    to {
      transform: translateX(100%);
      opacity: 0;
    }
  }
  
  .search-focused {
    box-shadow: 0 0 0 2px var(--md-accent-fg-color);
  }
  
  .active {
    color: var(--md-accent-fg-color) !important;
    font-weight: 600;
  }
  
  .toc-open .md-toc__list {
    display: block;
  }
  
  .menu-open .md-nav--primary {
    transform: translateX(0);
  }
  
  .feature-box {
    opacity: 0;
    transform: translateY(20px);
    transition: opacity 0.6s ease-out, transform 0.6s ease-out;
  }
  
  .dark-mode {
    filter: invert(1) hue-rotate(180deg);
  }
  
  .dark-mode img {
    filter: invert(1) hue-rotate(180deg);
  }
`;
document.head.appendChild(style);