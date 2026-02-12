#include "render_target.h"

#include "glad/include/glad/glad.h"

#include <print>

#include "aliasing.h"

namespace rc {

RenderTarget::RenderTarget(u64 width, u64 height) : texture_(width, height) {
  glGenFramebuffers(1, &frame_buffer_handle_);
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_handle_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         texture_.id(), 0);

  glGenRenderbuffers(1, &render_buffer_handle_);
  glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_handle_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, render_buffer_handle_);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::print("There was problem while generating a framebuffer\n");
  }
  BindDefault();
}

RenderTarget::~RenderTarget() {
  glDeleteRenderbuffers(1, &render_buffer_handle_);
  glDeleteFramebuffers(1, &frame_buffer_handle_);
}

void RenderTarget::Bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_handle_);
}

void RenderTarget::Clear(int flags) const {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(flags);
}

void RenderTarget::BindTexture(int texture_slot) const {
  texture_.BindTexture(texture_slot);
}

} // namespace rc
